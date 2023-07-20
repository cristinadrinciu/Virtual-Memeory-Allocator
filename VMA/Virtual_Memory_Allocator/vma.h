	#pragma once
	#include <inttypes.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>

	#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

	/* TODO : add your implementation for doubly-linked list */

	typedef struct node_t {
		struct node_t *prev;
		struct node_t *next;
		void *data;
	} node_t;

	typedef struct list_t {
		struct node_t *head;
		unsigned int size;
		unsigned int data_size;
	} list_t;

	typedef struct block_t {
		uint64_t start_address;
		size_t size;
		list_t *miniblock_list;  // works like list->head of miniblocks
	} block_t;

	typedef struct miniblock_t {
		uint64_t start_address;
		size_t size;
		uint8_t perm;
		void *rw_buffer;
	} miniblock_t;

	typedef struct arena_t {
		uint64_t arena_size;
		list_t *alloc_list;  // works like list->head of blocks
	} arena_t;

	void dll_add_nth_node(list_t *list, unsigned int n, const void *data);
	node_t *dll_remove_nth_node(list_t *list, unsigned int n);
	list_t *dll_create(unsigned int data_size);
	arena_t *alloc_arena(const uint64_t size);
	void dealloc_arena(arena_t *arena);
	// void add_node(list_t* list, unsigned int n, const void* data)
	void alloc_block(arena_t *arena, const uint64_t address,
					 const uint64_t size);
	void free_block(arena_t *arena, const uint64_t address);
	int verif_perm_write(arena_t *arena, const uint64_t address,
						 const uint64_t size);
	int verif_perm_read(arena_t *arena, const uint64_t address,
						const uint64_t size);
	int number_bytes(node_t *miniblock_node, const uint64_t address,
					 const uint64_t size);
	void read(arena_t *arena, uint64_t address, uint64_t size);
	void write(arena_t *arena, const uint64_t address, const uint64_t size,
			   int8_t *data);
	int total_miniblocks(const arena_t *arena);
	void perm_print(uint8_t perm);
	void pmap(const arena_t *arena);
	node_t *find_block(arena_t *arena, uint64_t address);
	void mprotect(arena_t *arena, uint64_t address, uint8_t *permission);
	int error_alloc_block(arena_t *arena, const uint64_t address,
						  const uint64_t size);
	int insert_node(arena_t *arena, const uint64_t address, block_t new_block);
