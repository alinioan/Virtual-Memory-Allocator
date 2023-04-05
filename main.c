#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vma.h"
#include "utils.h"

#define CMD_MAX_SIZE 300
#define MAX_ARGC 2

// reads the entire command with its argumnents from input
void read_command(char **command)
{
	int cmd_len;
	fgets(*command, sizeof(char) * CMD_MAX_SIZE, stdin);
	// removes the '\n' character when reading the command
	(*command)[strcspn(*command, "\n")] = 0;
	cmd_len = strlen(*command);
	*command = (char *)realloc(*command, cmd_len + 1);
	// realloc will always alloc less memory (cmd_len < CMD_MAX_SIZE)
	// so no defense needed
}

// splits the arguments into multiple one word strings
void get_arguments(char *my_argv[MAX_ARGC], size_t *my_argc, char *data)
{
	int i = 0;
	my_argv[i++] = strtok(NULL, " ");
	while (my_argv[i - 1] && i < MAX_ARGC)
		my_argv[i++] = strtok(NULL, " ");
	
	if (data) {
		char *rest = strtok(NULL, " ");
		// strncpy(data, rest, strlen(rest));
		while (rest) {
			strcat(data, rest);
			strcat(data, " ");
			rest = strtok(NULL, " ");
		}
		data[strlen(data) - 1] = '\0';
		*my_argc = 2;
	} else {
		if (my_argv[i - 1])
			i++;
		*my_argc = i - 1;
	}
}

int main(void)
{
	arena_t *arena;
	// my_argc doesn't count the command as an argument so if the input is
	// COMMAND <arg1> <arg2>, my_argc will be 2
	size_t my_argc = 0;
	char *command_line, *data = NULL, *command, *my_argv[MAX_ARGC] = {NULL};
	int exit_check = 0;

	while (exit_check == 0) {
		command_line = (char *)malloc(CMD_MAX_SIZE * sizeof(char));
		DIE(!command_line, "Command failed!");
		read_command(&command_line);
		// checks if it recived only a '\n'
		if (command_line[0] == '\0') {
			free(command_line);
			continue;
		}
		// get the main command and its args
		command = strtok(command_line, " ");
		if (strcmp(command, "WRITE") == 0) {
			data = malloc(CMD_MAX_SIZE * sizeof(char));
			data[0] = '\0';
		}
		get_arguments(my_argv, &my_argc, data);
		
		parse_command(&arena ,command, my_argv, my_argc, &exit_check, data);
		
		if (data) {
			free(data);
			data = NULL;
		}
		free(command_line);
	}
	return 0;
}
