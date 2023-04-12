#ifndef __UTILS_H_
#define __UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "vma.h"

// macro for handling error codes
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(errno);			        \
		}									\
	} while (0)

// macro for printing the invalid command error message
#define INVALID_COMMAND(assertion, argc, argc_, result)			\
	do {														\
		if ((assertion)) {										\
			printf("Invalid command. Please try again.\n");		\
			while ((argc)) {									\
				printf("Invalid command. Please try again.\n"); \
				(argc_)--;										\
			}													\
			result = 1;											\
		}														\
	} while (0)

void parse_command(arena_t **arena,
				   char *command, char *agrv[], size_t argc,
				   int *exit_check,
				   char *data);

#endif /* __UTILS_H_ */
