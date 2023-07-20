CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC = vma.c main.c commands.c
OBJ = $(SRC:.c=.o)

build: $(OBJ)
	$(CC) $(CFLAGS) -o vma $(OBJ)

run_vma: build
	./vma

clean:
	rm -f vma $(OBJ)

pack:
	zip -FSr Virtual_Memory_Allocator.zip README.md Makefile *.c *.h
