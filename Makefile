# Copyright 2023 Alexandru Alin-Ioan 312CA

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

# define targets
TARGETS=virtual_memory_alocator LinkedList parser

build: $(TARGETS) 
	$(CC) $(CFLAGS) main.c vma.o LinkedList.o parser.o -lm -o vma

virtual_memory_alocator:
	$(CC) $(CFLAGS) -c vma.c -o vma.o

LinkedList:
	$(CC) $(CFLAGS) -c LinkedList.c -o LinkedList.o

parser:
	$(CC) $(CFLAGS) -c parser.c -o parser.o

run:
	./vma

clean:
	rm vma *.o