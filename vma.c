// Copyright 2023 Alexandru Alin-Ioan 312CA
#include <stdlib.h>
#include <string.h>
#include "vma.h"
#include "LinkedList.h"
#include "utils.h"

#define PERMS_LEN 4

// alloc and init arena
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena;
	arena = malloc(sizeof(arena_t));
	DIE(!arena, "malloc failed!");
	arena->alloc_list = dll_create(sizeof(block_t));
	arena->arena_size = size;
	return arena;
}

// free arena, the data, the nodes, and the lists
void dealloc_arena(arena_t *arena)
{
	dll_node_t *crt = arena->alloc_list->head, *crt_mini;
	while (crt) {
		crt_mini = (*(block_t *)crt->data).miniblock_list->head;
		while (crt_mini) {
			free((*(miniblock_t *)crt_mini->data).rw_buffer);
			crt_mini = crt_mini->next;
		}
		dll_free(&(*(block_t *)crt->data).miniblock_list);
		crt = crt->next;
	}
	dll_free(&arena->alloc_list);
	free(arena);
}

// check if a new block to add overlaps with an existing one
int8_t check_overlap(arena_t *arena, const uint64_t address,
					 const uint64_t size)
{
	dll_node_t *crt = arena->alloc_list->head;
	while (crt) {
		block_t block = *(block_t *)crt->data;
		uint64_t end_address = block.start_address + block.size;
		if ((address >= block.start_address && address < end_address) ||
			(address + size > block.start_address &&
			address + size <= end_address))
			return 1;
		if ((block.start_address >= address &&
			 block.start_address < address + size) ||
			(end_address > address && end_address <= address + size))
			return 1;
		crt = crt->next;
	}
	return 0;
}

// alloc and init new block
block_t *init_block(const uint64_t address, const uint64_t size)
{
	block_t *block = malloc(sizeof(block_t));
	DIE(!block, "malloc failed!");
	block->size = size;
	block->start_address = address;
	block->miniblock_list = dll_create(sizeof(miniblock_t));
	return block;
}

// alloc and init new miniblock
miniblock_t *init_mini_block(const uint64_t address, const uint64_t size)
{
	miniblock_t *miniblock = malloc(sizeof(miniblock_t));
	DIE(!miniblock, "malloc failed!");
	miniblock->perm = 6;
	miniblock->size = size;
	miniblock->start_address = address;
	miniblock->rw_buffer = calloc(size, sizeof(int8_t));
	DIE(!miniblock->rw_buffer, "malloc failed!");
	return miniblock;
}

// adds a new block to the coresponding lists
void add_new_block(arena_t *arena,
				   const uint64_t address, const uint64_t size, size_t pos)
{
	block_t *new_block = init_block(address, size);
	DIE(!new_block, "malloc failed!");
	miniblock_t *new_mini = init_mini_block(address, size);
	DIE(!new_mini, "malloc failed!");

	dll_add_nth_node(new_block->miniblock_list, pos, new_mini);
	dll_add_nth_node(arena->alloc_list,	pos, new_block);
	free(new_block);
	free(new_mini);
}

// removes a block from the list and frees its data
void remove_block(arena_t *arena, dll_node_t *crt, size_t pos)
{
	dll_node_t *del;
	free((*(block_t *)crt->data).miniblock_list);
	free((block_t *)crt->data);
	del = dll_remove_nth_node(arena->alloc_list, pos + 1);
	free(del);
}

// add block exactly in between 2 other blocks
void add_and_merge(arena_t *arena, const uint64_t address, const uint64_t size,
				   block_t *next_block, block_t *crt_block, size_t pos,
				   dll_node_t *crt)
{
	miniblock_t *new_mini = init_mini_block(address, size);
	DIE(!new_mini, "malloc failed!");
	// add the miniblock at the end of the crt list
	dll_add_nth_node(crt_block->miniblock_list,
					 crt_block->miniblock_list->size + 1, new_mini);
	// append next list to crt list
	next_block->miniblock_list->head->prev = crt_block->miniblock_list->tail;
	crt_block->miniblock_list->tail->next = next_block->miniblock_list->head;
	crt_block->miniblock_list->tail = next_block->miniblock_list->tail;
	// change size of crt list
	crt_block->miniblock_list->size += next_block->miniblock_list->size;
	(*(block_t *)crt->data).size += size + next_block->size;
	// remove the crt->next node and free the memory
	remove_block(arena, crt->next, pos);
	free(new_mini);
}

// check if the block position is valid
uint8_t invalid_block(arena_t *arena,
					  const uint64_t address, const uint64_t size)
{
	if ((address) >= (arena)->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return 1;
	}
	if ((address) + (size) > (arena)->arena_size) {
		printf("The end address is past the size of the arena\n");
		return 1;
	}
	if (check_overlap((arena), (address), (size))) {
		printf("This zone was already allocated.\n");
		return 1;
	}
	return 0;
}

// alloc block and add it to the memory
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (invalid_block(arena, address, size))
		return;
	dll_node_t *crt;
	crt = arena->alloc_list->head;
	// add block in an empty list (the arena has no other blocks)
	if (!crt) {
		add_new_block(arena, address, size, 0);
		return;
	}
	size_t pos = 0;
	while (crt->next && address >
		   (*(block_t *)crt->next->data).start_address) {
		pos++;
		crt = crt->next;
	}
	if ((!crt->next && address > (*(block_t *)crt->data).start_address) ||
		crt->next)
		pos++;
	block_t next_block;
	block_t crt_block = *(block_t *)crt->data;
	if (crt->next)
		next_block = (*(block_t *)crt->next->data);
	// add block in an empty memory zone
	if (crt == arena->alloc_list->head &&
		address + size < crt_block.start_address) {
		add_new_block(arena, address, size, 0);
		return;
	}
	if (crt_block.start_address + crt_block.size < address ||
		(address + size < crt_block.start_address)) {
		add_new_block(arena, address, size, pos);
		return;
	}
	// add block exactly in between 2 other blocks
	if (crt->next) {
		if (crt_block.start_address + crt_block.size == address &&
			next_block.start_address == address + size) {
			add_and_merge(arena, address, size,
						  &next_block, &crt_block, pos, crt);
			return;
		}
	}
	// add block next to another block
	if (crt_block.start_address + crt_block.size == address) {
		// add at start of block
		miniblock_t *new_mini = init_mini_block(address, size);
		DIE(!new_mini, "malloc failed!");
		list_t *miniblock_list = (*(block_t *)crt->data).miniblock_list;
		dll_add_nth_node(miniblock_list, miniblock_list->size, new_mini);
		(*(block_t *)crt->data).size += size;
		free(new_mini);
		return;
	}
	if (crt->next || crt == arena->alloc_list->head) {
		// add at the back of block
		if (crt == arena->alloc_list->head)
			next_block = crt_block;
		if (next_block.start_address == address + size) {
			miniblock_t *new_mini = init_mini_block(address, size);
			DIE(!new_mini, "malloc failed!");
			list_t *miniblock_list = next_block.miniblock_list;
			dll_add_nth_node(miniblock_list, 0, new_mini);
			if (crt == arena->alloc_list->head) {
				(*(block_t *)crt->data).size += size;
				(*(block_t *)crt->data).start_address = address;
			} else {
				(*(block_t *)crt->next->data).size += size;
				(*(block_t *)crt->next->data).start_address = address;
			}
			free(new_mini);
			return;
		}
	}
}

// gets a block from the block list and its position in the list based on the
// address it is at
void get_block(dll_node_t **crt, block_t **prev_block, block_t **crt_block,
			   const uint64_t address, size_t *pos)
{
	while ((*crt)->next && address > (*crt_block)->start_address) {
		*crt = (*crt)->next;
		*crt_block = (block_t *)(*crt)->data;
		*pos += 1;
	}
	if (!(*crt)->next && address >= (*crt_block)->start_address) {
		*prev_block = *crt_block;
	} else if ((*crt)->prev) {
		*pos -= 1;
		*prev_block = (block_t *)(*crt)->prev->data;
	} else {
		*prev_block = *crt_block;
	}
}

// same as get_block but for miniblocks
void get_mini(list_t *miniblock_list, const uint64_t address,
			  dll_node_t **mini_crt, size_t *pos)
{
	(*mini_crt) = miniblock_list->head;
	miniblock_t *crt_mini = (miniblock_t *)(*mini_crt)->data;
	while ((*mini_crt)->next && address > crt_mini->start_address) {
		*mini_crt = (*mini_crt)->next;
		crt_mini = (miniblock_t *)(*mini_crt)->data;
		*pos = *pos + 1;
	}
}

// deletes the miniblock from the miniblock list and frees memory
void delete_miniblock(arena_t *arena, block_t *prev_block,
					  size_t pos_mini, size_t pos_block)
{
	dll_node_t *del_mini, *del_block;
	del_mini = dll_remove_nth_node(prev_block->miniblock_list, pos_mini);
	prev_block->size -= (*(miniblock_t *)del_mini->data).size;
	// free miniblock buffer
	if ((*(miniblock_t *)del_mini->data).rw_buffer)
		free((*(miniblock_t *)del_mini->data).rw_buffer);
	if (prev_block->miniblock_list->size == 0) {
		// remove block if it remains empty
		dll_free(&prev_block->miniblock_list);
		del_block = dll_remove_nth_node(arena->alloc_list, pos_block);
		free(del_block->data);
		free(del_block);
	} else {
		if (pos_mini == 0) {
			prev_block->start_address =
		(*(miniblock_t *)prev_block->miniblock_list->head->data).start_address;
		}
	}
	free(del_mini->data);
	free(del_mini);
}

// frees block from memory
void free_block(arena_t *arena, const uint64_t address)
{
	if (!arena->alloc_list->head) {
		printf("Invalid address for free.\n");
		return;
	}
	dll_node_t *crt = arena->alloc_list->head, *mini_crt;
	block_t *crt_block = (block_t *)crt->data, *prev_block;
	size_t pos_block = 0, pos_mini = 0;
	get_block(&crt, &prev_block, &crt_block, address, &pos_block);
	if (crt == arena->alloc_list->head && address < crt_block->start_address) {
		printf("Invalid address for free.\n");
		return;
	}
	if (address >= prev_block->start_address + prev_block->size) {
		printf("Invalid address for free.\n");
		return;
	}
	get_mini(prev_block->miniblock_list, address, &mini_crt, &pos_mini);
	if (address > (*(miniblock_t *)mini_crt->data).start_address &&
		address < (*(miniblock_t *)mini_crt->data).start_address +
		(*(miniblock_t *)mini_crt->data).size) {
		printf("Invalid address for free.\n");
		return;
	}
	if (address != (*(miniblock_t *)mini_crt->data).start_address)  {
		printf("Invalid address for free.\n");
		return;
	}
	// if mini is first in list or last in list
	if (pos_mini == 0 || pos_mini == prev_block->miniblock_list->size - 1) {
		delete_miniblock(arena, prev_block, pos_mini, pos_block);
		return;
	}
	// if mini is in the middle
	if (pos_mini > 0 && pos_mini < prev_block->miniblock_list->size - 1) {
		uint64_t new_adr = (*(miniblock_t *)mini_crt->next->data).start_address;
		uint64_t new_size = 0;
		dll_node_t *i = mini_crt->next;
		while (i) {
			new_size += (*(miniblock_t *)i->data).size;
			i = i->next;
		}
		// create new block and init its miniblock list with the
		// end of the previous list
		block_t *new_block = init_block(new_adr, new_size);
		new_block->miniblock_list->size = prev_block->miniblock_list->size
		- pos_mini - 1;
		new_block->miniblock_list->head = mini_crt->next;
		new_block->miniblock_list->tail = prev_block->miniblock_list->tail;
		new_block->miniblock_list->head->prev = NULL;

		// add new block to block list
		dll_add_nth_node(arena->alloc_list, pos_block + 1, new_block);

		// remove the new_block list from the previous list
		prev_block->miniblock_list->tail = mini_crt;
		prev_block->miniblock_list->tail->next = NULL;
		prev_block->miniblock_list->size = pos_mini + 1;
		prev_block->size -= new_size;
		delete_miniblock(arena, prev_block, pos_mini, pos_block);

		free(new_block);
		return;
	}
}

// check prems for all the miniblocks to write/read
int8_t check_perms(dll_node_t *mini_crt, miniblock_t *mini,
				   const uint64_t address, const uint64_t size,
				   int8_t (*check)(), char *r_w)
{
	size_t size_written = mini->start_address - address;
	dll_node_t *i = mini_crt;
	while (size_written < size && i) {
		if (check(mini) == 0) {
			printf("Invalid permissions for %s.\n", r_w);
			return 0;
		}
		size_written += mini->size;
		i = i->next;
		if (i)
			mini = (miniblock_t *)i->data;
	}
	return 1;
}

// read prems condition
int8_t check_read_perms(miniblock_t *miniblock)
{
	if (miniblock->perm != 4 && miniblock->perm != 5 &&
		miniblock->perm != 6 && miniblock->perm != 7)
		return 0;
	return 1;
}

// read data from minblocks
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	if (!arena->alloc_list->head) {
		printf("Invalid address for read.\n");
		return;
	}
	if (address >= arena->arena_size) {
		printf("Invalid address for read.\n");
		return;
	}
	dll_node_t *crt = arena->alloc_list->head, *mini_crt;
	block_t *crt_block = (block_t *)crt->data, *prev_block;
	size_t pos_block = 0, pos_mini = 0;
	get_block(&crt, &prev_block, &crt_block, address, &pos_block);

	if (address >= prev_block->start_address + prev_block->size) {
		printf("Invalid address for read.\n");
		return;
	}
	if (address + size > prev_block->start_address + prev_block->size)
		printf("Warning: size was bigger than the block size. "
			   "Reading %ld characters.\n",
			   prev_block->start_address - address + prev_block->size);
	get_mini(prev_block->miniblock_list, address, &mini_crt, &pos_mini);
	dll_node_t *i = mini_crt;
	int64_t size_written = 0, read_start_address;
	miniblock_t *mini = (miniblock_t *)mini_crt->data;
	int8_t *data = calloc(2 * size, sizeof(int8_t));
	int8_t *src_buff, *dest_buff;
	size_t size_buff;
	if (address < mini->start_address) {
		mini_crt = mini_crt->prev;
		mini = (miniblock_t *)mini_crt->data;
	}
	if (check_perms(mini_crt, mini, address,
					size, check_read_perms, "read") == 0) {
		free(data);
		return;
	}
	if (address < mini->start_address) {
		mini_crt = mini_crt->prev;
		mini = (miniblock_t *)mini_crt->data;
	}
	// size_written until now
	size_written = 0;
	// the position where the reading starts in the curent miniblock
	read_start_address = address - mini->start_address;
	i = mini_crt;
	while (size_written < (int64_t)size && i) {
		src_buff = (int8_t *)mini->rw_buffer + read_start_address;
		dest_buff = data + size_written;
		size_buff = mini->size - read_start_address;
		if (size_buff > size - size_written)
			size_buff = size - size_written;
		memcpy(dest_buff, src_buff, size_buff);
		size_written += size_buff;
		i = i->next;
		if (i)
			mini = (miniblock_t *)i->data;
		read_start_address = 0;
	}
	data[size] = 0;
	printf("%s\n", data);
	free(data);
}

// write prems condition
int8_t check_write_perms(miniblock_t *miniblock)
{
	if (miniblock->perm != 2 && miniblock->perm != 3 &&
		miniblock->perm != 6 && miniblock->perm != 7)
		return 0;
	return 1;
}

// write data in miniblocks
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data)
{
	if (!arena->alloc_list->head) {
		printf("Invalid address for write.\n");
		return;
	}
	if (address >= arena->arena_size) {
		printf("Invalid address for write.\n");
		return;
	}
	dll_node_t *crt = arena->alloc_list->head, *mini_crt;
	block_t *crt_block = (block_t *)crt->data, *prev_block;
	size_t pos_block = 0, pos_mini = 0;
	get_block(&crt, &prev_block, &crt_block, address, &pos_block);

	if (address >= prev_block->start_address + prev_block->size) {
		printf("Invalid address for write.\n");
		return;
	}
	if (address + size > prev_block->start_address + prev_block->size)
		printf("Warning: size was bigger than the block size. "
			   "Writing %ld characters.\n",
			   prev_block->start_address - address + prev_block->size);
	get_mini(prev_block->miniblock_list, address, &mini_crt, &pos_mini);
	dll_node_t *i;
	int64_t size_written = 0, write_start_address;
	miniblock_t *mini = (miniblock_t *)mini_crt->data;
	int8_t *src_buff, *dest_buff;
	size_t size_buff;
	if (address < mini->start_address) {
		mini_crt = mini_crt->prev;
		mini = (miniblock_t *)mini_crt->data;
	}
	if (check_perms(mini_crt, mini, address,
					size, check_write_perms, "write") == 0)
		return;

	if (address < mini->start_address) {
		mini_crt = mini_crt->prev;
		mini = (miniblock_t *)mini_crt->data;
	}
	// size_written until now
	size_written = 0;
	// the position where the writting starts in the curent miniblock
	write_start_address = address - mini->start_address;
	i = mini_crt;
	while (size_written < (int64_t)size && i) {
		dest_buff = (int8_t *)mini->rw_buffer + write_start_address;
		src_buff = data + size_written;
		size_buff = mini->size - write_start_address;
		if (size_buff > size - size_written)
			size_buff = size - size_written;
		memcpy(dest_buff, src_buff, size_buff);
		size_written += size_buff;
		i = i->next;
		if (i)
			mini = (miniblock_t *)i->data;
		write_start_address = 0;
	}
}

// gets the information for the begining of the pmap output
void get_pmap_info(const arena_t *arena,
				   uint64_t *total_mem, uint64_t *free_mem,
				   uint64_t *allocated_blocks, uint64_t *allocated_minis)
{
	size_t total_block_size = 0;
	*total_mem = arena->arena_size;
	*allocated_blocks = arena->alloc_list->size;
	*allocated_minis = 0;
	dll_node_t *crt = arena->alloc_list->head;
	while (crt) {
		total_block_size += (*(block_t *)crt->data).size;
		*allocated_minis += (*(block_t *)crt->data).miniblock_list->size;
		crt = crt->next;
	}
	*free_mem = *total_mem - total_block_size;
}

// transform prem form int to string
void get_perms_string(uint8_t perm, char *perm_str)
{
	switch (perm) {
	case 0:
		strncpy(perm_str, "---", PERMS_LEN);
		break;
	case 1:
		strncpy(perm_str, "--X", PERMS_LEN);
		break;
	case 2:
		strncpy(perm_str, "-W-", PERMS_LEN);
		break;
	case 3:
		strncpy(perm_str, "-WX", PERMS_LEN);
		break;
	case 4:
		strncpy(perm_str, "R--", PERMS_LEN);
		break;
	case 5:
		strncpy(perm_str, "R-X", PERMS_LEN);
		break;
	case 6:
		strncpy(perm_str, "RW-", PERMS_LEN);
		break;
	case 7:
		strncpy(perm_str, "RWX", PERMS_LEN);
		break;
	default:
		break;
	}
}

// print the miniblock list with the pmap format
void pmap_print_miniblocks(list_t *miniblock_list)
{
	char perms[PERMS_LEN];
	dll_node_t *crt = miniblock_list->head;
	size_t pos = 1;
	uint64_t start_address, end_address;
	while (crt) {
		start_address = (*(miniblock_t *)crt->data).start_address;
		end_address = start_address + (*(miniblock_t *)crt->data).size;
		get_perms_string((*(miniblock_t *)crt->data).perm, perms);
		printf("Miniblock %ld:\t\t0x%lX\t\t-\t\t0x%lX\t\t| %s\n",
			   pos, start_address, end_address, perms);
		pos++;
		crt = crt->next;
	}
}

// print the proccess map
void pmap(const arena_t *arena)
{
	uint64_t total_mem, free_mem, allocated_blocks, allocated_minis;
	get_pmap_info(arena, &total_mem, &free_mem,
				  &allocated_blocks, &allocated_minis);
	printf("Total memory: 0x%lX bytes\n"
		   "Free memory: 0x%lX bytes\n"
		   "Number of allocated blocks: %ld\n"
		   "Number of allocated miniblocks: %ld\n",
		   total_mem, free_mem, allocated_blocks, allocated_minis);

	dll_node_t *crt = arena->alloc_list->head;
	size_t pos = 1;
	uint64_t start_address, end_address;
	// go thorugh block list
	while (crt) {
		start_address = (*(block_t *)crt->data).start_address;
		end_address = start_address + (*(block_t *)crt->data).size;
		printf("\nBlock %ld begin\n"
			   "Zone: 0x%lX - 0x%lX\n",
			   pos, start_address, end_address);
		pmap_print_miniblocks((*(block_t *)crt->data).miniblock_list);
		printf("Block %ld end\n", pos);
		pos++;
		crt = crt->next;
	}
}

// get perms from string to int
int8_t get_perms_int(int8_t *permissions)
{
	uint8_t total = 0;
	char *str = (char *)permissions, *perm;
	perm = strtok(str, " ");
	while (perm) {
		if (strcmp(perm, "PROT_READ") == 0)
			total += 4;
		if (strcmp(perm, "PROT_WRITE") == 0)
			total += 2;
		if (strcmp(perm, "PROT_EXEC") == 0)
			total += 1;
		perm = strtok(NULL, " ");
	}
	return total;
}

// change permissions on miniblocks
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	if (!arena->alloc_list->head) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	if (address >= arena->arena_size) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	uint8_t perm = get_perms_int(permission);

	dll_node_t *crt = arena->alloc_list->head, *mini_crt;
	block_t *crt_block = (block_t *)crt->data, *prev_block;
	size_t pos_block = 0, pos_mini = 0;
	get_block(&crt, &prev_block, &crt_block, address, &pos_block);
	if (crt == arena->alloc_list->head && address < crt_block->start_address) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	if (address >= prev_block->start_address + prev_block->size) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	get_mini(prev_block->miniblock_list, address, &mini_crt, &pos_mini);
	if (address > (*(miniblock_t *)mini_crt->data).start_address &&
		address < (*(miniblock_t *)mini_crt->data).start_address +
		(*(miniblock_t *)mini_crt->data).size) {
		printf("Invalid address for mprotect.\n");
		return;
	}
	(*(miniblock_t *)mini_crt->data).perm = perm;
}
