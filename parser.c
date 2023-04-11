#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vma.h"
#include "utils.h"

arena_t *p_alloc_arena(char *argv[], size_t argc)
{
	if (argc != 1) {
		printf("Invalid command. Please try again.\n");
		while (argc) {
			printf("Invalid command. Please try again.\n");
			argc--;
		}
		return NULL;
	}
	arena_t *arena;
	uint64_t arena_size = atol(argv[0]);
	arena = alloc_arena(arena_size);
	return arena;
}

void p_dealloc_arena(arena_t *arena, size_t argc, int *exit_check)
{
	INVALID_COMMAND(argc != 0, argc);
	*exit_check = 1;
	dealloc_arena(arena);
}

void p_alloc_block(arena_t *arena, char *argv[], size_t argc)
{
	INVALID_COMMAND(argc != 2, argc);
	uint64_t adress, size;
	adress = atol(argv[0]);
	size = atol(argv[1]);
	alloc_block(arena, adress, size);
}

void p_free_block(arena_t *arena, char *argv[], size_t argc)
{
	INVALID_COMMAND(argc != 1, argc);
	uint64_t adress = atol(argv[0]);
	free_block(arena, adress);
}

void p_read(arena_t *arena, char *argv[], size_t argc)
{
	INVALID_COMMAND(argc != 2, argc);
	uint64_t adress, size;
	adress = atol(argv[0]);
	size = atol(argv[1]);
	read(arena, adress, size);
}

void p_write(arena_t *arena, char *argv[], size_t argc, int8_t *data)
{
	INVALID_COMMAND(argc != 2, argc);
	uint64_t adress, size;
	adress = atol(argv[0]);
	size = atol(argv[1]);
	printf("%s", data);
	write(arena, adress, size, data);
}

void p_pmap(arena_t *arena, size_t argc)
{
	INVALID_COMMAND(argc != 0, argc);
	pmap(arena);
}

void parse_command(arena_t **arena,
				   char *command, char *argv[], size_t argc,
				   int *exit_check,
				   char *data)
{
	// command parser
	if (strcmp(command, "ALLOC_ARENA") == 0) {
		*arena = p_alloc_arena(argv, argc);
	} else if (strcmp(command, "ALLOC_BLOCK") == 0) {
		p_alloc_block(*arena, argv, argc);
	} else if (strcmp(command, "FREE_BLOCK") == 0) {
		p_free_block(*arena, argv, argc);
	} else if (strcmp(command, "READ") == 0) {
		p_read(*arena, argv, argc);
	} else if (strcmp(command, "WRITE") == 0) {
		p_write(*arena, argv, argc, (int8_t *)data);
	} else if (strcmp(command, "PMAP") == 0) {
		p_pmap(*arena, argc);
	} else if (strcmp(command, "DEALLOC_ARENA") == 0) {
		p_dealloc_arena(*arena, argc, exit_check);
	} else {
		printf("Invalid command. Please try again.\n");
		while (argc) {
			printf("Invalid command. Please try again.\n");
			argc--;
		}
		char *rest = strtok(NULL, " ");
		while (rest) {
			printf("Invalid command. Please try again.\n");
			rest = strtok(NULL, " ");
		}
	}
}
