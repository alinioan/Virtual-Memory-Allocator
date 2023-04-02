# Alexandru Alin-Ioan 312CA 2023

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

# define targets
TARGETS=vma

build: $(TARGETS) 
	$(CC) $(CFLAGS) main.c vma.o -lm -o virtual_memory_alocator

vma:
	$(CC) $(CFLAGS) -c vma.c -o vma.o

clean:
	rm image_editor *.o