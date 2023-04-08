#include <stdlib.h>
#include "vma.h"
#include "LinkedList.h"
#include "utils.h"

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena;
	arena = malloc(sizeof(arena_t));
	DIE(!arena, "malloc failed!");
	arena->alloc_list = dll_create(sizeof(block_t));
	arena->arena_size = size;
	return arena;
}

void dealloc_arena(arena_t *arena)
{
	dll_node_t* crt = arena->alloc_list->head;
	while(crt) {
		dll_free(&(*(block_t*)crt->data).miniblock_list);
		crt = crt->next;
	}
	dll_free(&arena->alloc_list);
	free(arena);
}

block_t* init_block(const uint64_t address, const uint64_t size)
{
	block_t* block = malloc(sizeof(block_t));
	if (!block)
		return NULL;
	block->size = size;
	block->start_address = address;
	block->miniblock_list = dll_create(sizeof(miniblock_t));
	return block;
}

miniblock_t* init_mini_block(const uint64_t address, const uint64_t size)
{
	miniblock_t* miniblock = malloc(sizeof(miniblock_t));
	if (!miniblock)
		return NULL;
	miniblock->perm = 6;
	miniblock->size = size;
	miniblock->start_address = address;
	return miniblock;
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	INVALID_BLOCK(address, size, arena->arena_size);
	// TODO: add check for overlaping memory
	dll_node_t *crt;
	crt = arena->alloc_list->head;
	// add block in an empty list (the arena has no other blocks)
	if (!crt) {
		block_t *new_block = init_block(address, size);
		DIE(!new_block, "malloc failed!");
		miniblock_t* new_mini = init_mini_block(address, size);
		DIE(!new_mini, "malloc failed!");

		dll_add_nth_node(new_block->miniblock_list, 0, new_mini);
		dll_add_nth_node(arena->alloc_list,	0, new_block);
		free(new_block);
		free(new_mini);
		return;
	}
	size_t pos = 0;
	while (crt->next && address > (*(block_t*)crt->next->data).start_address) {
		pos++;
		crt = crt->next; 
	}
	if (!crt->next && address > (*(block_t*)crt->data).start_address)
		pos++;
	block_t next_block;
	block_t crt_block = *(block_t *)crt->data;
	if (crt->next)
		next_block = (*(block_t*)crt->next->data);
	
	// add block in an empty memory zone
	if (crt_block.start_address + crt_block.size + 1 < address) {
		block_t *new_block = init_block(address, size);
		DIE(!new_block, "malloc failed!");
		miniblock_t* new_mini = init_mini_block(address, size);
		DIE(!new_mini, "malloc failed!");

		dll_add_nth_node(new_block->miniblock_list, pos, new_mini);
		dll_add_nth_node(arena->alloc_list,	pos, new_block);
		free(new_block);
		free(new_mini);
		return;	
	}
	// add block exactly in between 2 other blocks
	if (crt->next) {
		if (crt_block.start_address + crt_block.size + 1 == address &&
			next_block.start_address == address + size) {
			miniblock_t* new_mini = init_mini_block(address, size);
			DIE(!new_mini, "malloc failed!");
			// add new miniblock to crt list
			list_t *miniblock_list = (*(block_t*)crt->data).miniblock_list;
			dll_add_nth_node(miniblock_list, miniblock_list->size, new_mini);
			// append next list to crt list
			(*(block_t*)crt->data).size += size + next_block.size;
			miniblock_list->tail->next = next_block.miniblock_list->head;
			next_block.miniblock_list->head->prev = miniblock_list->tail;
			free(next_block.miniblock_list);
			free(new_mini);
			return;
		}
	}
	// add block next to another block
	if (crt_block.start_address + crt_block.size + 1 == address) {
		miniblock_t* new_mini = init_mini_block(address, size);
		DIE(!new_mini, "malloc failed!");

		list_t *miniblock_list = (*(block_t*)crt->data).miniblock_list;
		dll_add_nth_node(miniblock_list, miniblock_list->size, new_mini);
		(*(block_t*)crt->data).size += size;
		free(new_mini);
		return;
	}
	if (crt->next || crt == arena->alloc_list->head) {
		if (next_block.start_address == address + size) {
			miniblock_t* new_mini = init_mini_block(address, size);
			DIE(!new_mini, "malloc failed!");

			list_t *miniblock_list = (*(block_t*)crt->next->data).miniblock_list;
			dll_add_nth_node(miniblock_list, 0, new_mini);

			(*(block_t*)crt->next->data).size += size;
			(*(block_t*)crt->next->data).start_address = address;
			free(new_mini);
			return;	
		}
	}
}

void free_block(arena_t *arena, const uint64_t address)
{

}

void read(arena_t *arena, uint64_t address, uint64_t size)
{

}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{

}

void get_pmap_info(const arena_t* arena,
				   uint64_t *total_mem, uint64_t *free_mem,
				   uint64_t *allocated_blocks, uint64_t *allocated_minis)
{
	size_t total_block_size = 0;
	*total_mem = arena->arena_size;
	*allocated_blocks = arena->alloc_list->size;
	*allocated_minis = 0;
	dll_node_t *crt = arena->alloc_list->head;
	while (crt) {
		total_block_size += (*(block_t*)crt->data).size;
		*allocated_minis += (*(block_t*)crt->data).miniblock_list->size;
		crt = crt->next;
	}
	*free_mem = *total_mem - total_block_size;
}

void pmap_print_miniblocks(list_t* miniblock_list)
{
	dll_node_t *crt = miniblock_list->head;
	size_t pos = 1;
	uint64_t start_address, end_address;
	while (crt) {
		start_address = (*(miniblock_t*)crt->data).start_address;
		end_address = start_address + (*(miniblock_t*)crt->data).size;

		printf("Miniblock %d:\t\t0x%X\t\t-\t\t0x%X\t\t| %d\n",
			   pos, start_address, end_address, (*(miniblock_t*)crt->data).perm);
		pos++;
		crt = crt->next;
	}
}

void pmap(const arena_t *arena)
{
	uint64_t total_mem, free_mem, allocated_blocks, allocated_minis;
	get_pmap_info(arena, &total_mem, &free_mem,
				  &allocated_blocks, &allocated_minis);
	printf("Total memory: 0x%X\n"
		   "Free memory: 0x%X\n"
		   "Number of allocated blocks: %d\n"
		   "Number of allocated miniblocks: %d\n",
		   total_mem, free_mem, allocated_blocks, allocated_minis);

	dll_node_t *crt = arena->alloc_list->head;
	size_t pos = 1;
	uint64_t start_address, end_address;
	while (crt) {
		start_address = (*(block_t*)crt->data).start_address;
		end_address = start_address + (*(block_t*)crt->data).size;
		printf("\nBlock %d begin\n"
			   "Zone: 0x%X - 0x%X\n",
			   pos, start_address, end_address);
		pmap_print_miniblocks((*(block_t*)crt->data).miniblock_list);
		printf("Block %d end\n", pos);
		pos++;
		crt = crt->next;
	}

}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}
