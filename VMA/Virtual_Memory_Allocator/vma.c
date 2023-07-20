	#include "vma.h"

	list_t *dll_create(unsigned int data_size)
{
		list_t *list = malloc(sizeof(list_t));
		DIE(!list, "Failed to malloc!\n");
		list->head = NULL;
		list->size = 0;
		list->data_size = data_size;
		return list;
}

	arena_t *alloc_arena(const uint64_t size)
{
		arena_t *arena = malloc(sizeof(arena_t));  // get space for the arena
		DIE(!arena, "Failed to malloc!\n");
		arena->arena_size = size;
		arena->alloc_list = dll_create(sizeof(block_t));
		// at first, the arena is empty and points to NULL
		return arena;
}

	void free_list(list_t **pp_list)  // general function that frees a list
{
		if (!(*pp_list)->head) {
			free(*pp_list);
			return;
		}
		node_t *curr;
		curr = (*pp_list)->head;
		while (curr) {
			node_t *trash = curr;
			curr = curr->next;
			free(trash->data);
			free(trash);
		}
		free(*(pp_list));
		*pp_list = NULL;
}

	node_t *find_block(arena_t *arena, uint64_t address)
{
		node_t *block_node = arena->alloc_list->head;
		while (block_node) {
			node_t *miniblock_node =
				((block_t *)block_node->data)->miniblock_list->head;
			while (miniblock_node) {
				if (address >=
						((miniblock_t *)miniblock_node->data)->start_address &&
					address <
					((miniblock_t *)miniblock_node->data)->start_address +
								((miniblock_t *)miniblock_node->data)->size)
					return miniblock_node;
				miniblock_node = miniblock_node->next;
			}
			block_node = block_node->next;
		}
		return NULL;
}

	void dealloc_arena(arena_t *arena)
{
		// the arena is empty
		if (arena->alloc_list->size == 0) {
			free(arena->alloc_list);
			free(arena);
			arena = NULL;
			return;
		}

		node_t *block_node = arena->alloc_list->head;
		while (block_node->next) {
			node_t *miniblock_node =
				((block_t *)block_node->data)->miniblock_list->head;
			while (miniblock_node) {
				free(((miniblock_t *)miniblock_node->data)->rw_buffer);
				miniblock_node = miniblock_node->next;
			}
			free_list(&(((block_t *)block_node->data)->miniblock_list));
			block_node = block_node->next;
		}
		node_t *miniblock_node =
			((block_t *)block_node->data)->miniblock_list->head;
		while (miniblock_node) {
			free(((miniblock_t *)miniblock_node->data)->rw_buffer);
			miniblock_node = miniblock_node->next;
		}
		free_list(&((block_t *)block_node->data)->miniblock_list);
		free_list(&arena->alloc_list);
		free(arena);
		arena = NULL;
}

	void dll_add_nth_node(list_t *list, unsigned int n, const void *data)
{
		node_t *node;
		if (list->size == 0) {
			node = malloc(sizeof(node_t));
			DIE(!node, "Failed to malloc!\n");
			node->next = NULL;
			node->prev = NULL;
			node->data = malloc(list->data_size);
			DIE(!node->data, "Failed to malloc!\n");
			memcpy(node->data, data, list->data_size);
			node->next = NULL;
			node->prev = NULL;
			list->head = node;
			list->size++;
		} else if (n == 0) {
			node = malloc(sizeof(node_t));
			DIE(!node, "Failed to malloc!\n");
			node->next = NULL;
			node->prev = NULL;
			node->data = malloc(list->data_size);
			DIE(!node->data, "Failed to malloc!\n");
			memcpy(node->data, data, list->data_size);
			node->next = list->head;
			node->prev = NULL;
			list->head->prev = node;
			list->head = node;
			list->size++;
		} else if (n >= list->size) {
			node_t *curr;
			node = malloc(sizeof(node_t));
			DIE(!node, "Failed to malloc!\n");
			node->next = NULL;
			node->prev = NULL;
			node->data = malloc(list->data_size);
			DIE(!node->data, "Failed to malloc!\n");
			memcpy(node->data, data, list->data_size);
			curr = list->head;
			while (curr->next)
				curr = curr->next;
			curr->next = node;
			node->prev = curr;
			node->next = NULL;
			list->size++;
		} else if (n > 0 && n < list->size) {
			node_t *curr;
			curr = list->head;
			node = malloc(sizeof(node_t));
			DIE(!node, "Failed to malloc!\n");
			node->next = NULL;
			node->prev = NULL;
			node->data = malloc(list->data_size);
			DIE(!node->data, "Failed to malloc!\n");
			memcpy(node->data, data, list->data_size);
			for (unsigned int i = 0; i < n - 1; i++)
				curr = curr->next;
			node->next = curr->next;
			node->prev = curr;
			curr->next = node;
			node->next->prev = node;
			list->size++;
		}
}

	node_t *dll_remove_nth_node(list_t *list, unsigned int n)
{
		if (list->size == 0)
			return NULL;
		if (n >= list->size - 1)
			n = list->size - 1;
		if (n == 0) {
			node_t *aux = list->head;
			if (list->size > 1) {
				list->head = list->head->next;
				list->head->prev = NULL;
			} else {
				list->head = NULL;
			}
			list->size--;
			return aux;
		}
		node_t *current = list->head;
		while (n--)
			current = current->next;
		node_t *aux = current;
		if (!current->next) {
			node_t *prev_node = current->prev;
			prev_node->next = NULL;
			list->size--;
			return aux;
		}
		node_t *prev_node = current->prev;
		node_t *next_node = current->next;
		prev_node->next = current->next;
		next_node->prev = current->prev;
		list->size--;
		return aux;
}

	void connect_blocks(arena_t *arena, node_t *block_node, int index)
{
		// connect the miniblock list
		node_t *prev_miniblock =
			((block_t *)block_node->data)->miniblock_list->head;

		// get to the last miniblock of the prev's node
		while (prev_miniblock->next)
			prev_miniblock = prev_miniblock->next;

		prev_miniblock->next =
			((block_t *)(block_node->next->data))->miniblock_list->head;
		((block_t *)(block_node->next->data))->miniblock_list->head->prev =
			prev_miniblock;

		((block_t *)(block_node->data))->miniblock_list->size +=
			((block_t *)(block_node->next->data))->miniblock_list->size;

		// resize the block
		((block_t *)block_node->data)->size +=
			((block_t *)block_node->next->data)->size;

		// connect the block nodes in the list of blocks
		node_t *trash_node = dll_remove_nth_node(arena->alloc_list, index + 1);

		// free the node and block
		free(((block_t *)trash_node->data)->miniblock_list);
		free(trash_node->data);
		free(trash_node);
}

	int error_alloc_block(arena_t *arena, const uint64_t address,
						  const uint64_t size)
{
		node_t *block_node =
			arena->alloc_list->head;  // a node from the list of blocks
		// verify if the given address is valid
		if (address >= arena->arena_size) { // is out of the arena
			printf("The allocated address is outside the size of arena\n");
			return 0;
		}

		// verify if there is already a block on the given address
		if (arena->alloc_list->size != 0 && address +
			size <= arena->arena_size) {
		// if the list of blocks is not empty
			while (block_node) {
				// block_node = arena->alloc_list->head;
				// initiate with the head of the list
				block_t block =
					*(block_t *)
						block_node->data;  // the date of the node is a block
				uint64_t start_block = block.start_address;
				if (start_block + block.size - 1 >= address &&
					start_block <= address + size - 1)
				// checks if the given address if before the start of the block
				//  or is after and the sizes do not allow to allocate
				{
					printf("This zone was already allocated.\n");
					return 0;
				}
				block_node = block_node->next;
			}
		}

		if (address + size > arena->arena_size) {
			printf("The end address is past the size of the arena\n");
			return 0;
		}
		return 1;
}

	int insert_node(arena_t *arena, const uint64_t address, block_t new_block)
{
		if (!arena->alloc_list->head) { // the arena is empty
			dll_add_nth_node(arena->alloc_list, 0, &new_block);
			return 0;
		}

		if (address < ((block_t *)arena->alloc_list->head->data)->start_address)
		// add at the beggining of the list if the address is before
		// the first node's address
		{
			dll_add_nth_node(arena->alloc_list, 0, &new_block);
		}

		// initiate a node to get to the final block of the list in the arena
		node_t *last;
		last = arena->alloc_list->head;
		while (last->next)
			last = last->next;
		uint64_t finish_address =
			((block_t *)last->data)->start_address +
			((block_t *)last->data)->size;

		if (address >=
			finish_address)  // add at the end of the block list if the address
		// if after the end of the last block of the list
		{
			dll_add_nth_node(arena->alloc_list, arena->alloc_list->size + 1,
							 &new_block);
		}

		if (address >
			((block_t *)arena->alloc_list->head->data)->start_address &&
			address < finish_address)
		// add in the list
		{
			// get to the node next to we put the new node
			node_t *curr;
			int index = 0;
			curr = arena->alloc_list->head;
			while ((((block_t *)curr->next->data)->start_address +
					((block_t *)curr->next->data)->size) <= address) {
				curr = curr->next;
				index++;
			}
			node_t *new_node = malloc(sizeof(node_t));
			DIE(!new_node, "Failed to malloc!\n");
			new_node->data = malloc(sizeof(block_t));
			DIE(!new_node->data, "Failed to malloc!\n");
			memcpy(new_node->data, &new_block, sizeof(block_t));
			new_node->next = curr->next;
			curr->next->prev = new_node;
			curr->next = new_node;
			new_node->prev = curr;
			arena->alloc_list->size++;
		}
		return 1;
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
		node_t *block_node;
		int stop = error_alloc_block(arena, address, size);
		if (stop == 0)
			return;

		// make a new block
		block_t new_block;
		new_block.start_address = address;
		new_block.size = size;
		new_block.miniblock_list = dll_create(sizeof(miniblock_t));

		// make a new miniblock
		miniblock_t new_miniblock;
		new_miniblock.size = size;
		new_miniblock.size = size;
		new_miniblock.start_address = address;
		new_miniblock.perm = 6;
		new_miniblock.rw_buffer = calloc(size, 1);

		dll_add_nth_node(new_block.miniblock_list, 0, &new_miniblock);

		int insert = insert_node(arena, address, new_block);
		if (insert == 0)
			return;

		// make the modification of transforming tangent blocks in miniblocks
		block_node = arena->alloc_list->head;
		int index = 0;
		while (((block_t *)block_node->data)->start_address != address) {
			block_node = block_node->next;
			index++;
		}

		if (block_node->prev && block_node->next &&
			address + ((block_t *)block_node->data)->size ==
				((block_t *)block_node->next->data)->start_address &&
			((block_t *)block_node->prev->data)->start_address +
					((block_t *)block_node->prev->data)->size ==
				address) {
			connect_blocks(arena, block_node, index + 1);
			connect_blocks(arena, block_node->prev, index - 1);
			return;
		}

		block_node = arena->alloc_list->head;
		index = 0;
		while ((((block_t *)block_node->next->data)->start_address +
				((block_t *)block_node->next->data)->size) <= address) {
			block_node = block_node->next;
			index++;
		}
		if ((((block_t *)block_node->data)->start_address +
			((block_t *)block_node->data)->size) == address)
		// if the added block and its prev are tangent
		{
			connect_blocks(arena, block_node, index);
			return;
		}

		block_node = arena->alloc_list->head;
		index = 0;
		while (((block_t *)block_node->data)->start_address != address) {
			block_node = block_node->next;
			index++;
		}

		if (block_node->next &&
			((block_t *)block_node->data)->start_address +
					((block_t *)block_node->data)->size ==
				((block_t *)block_node->next->data)->start_address)
		// if the added block and its next are tangent
		{
			connect_blocks(arena, block_node, index);
			return;
		}
}

	void free_block(arena_t *arena, const uint64_t address)
{
		if (address >= arena->arena_size || !find_block(arena, address))
		// verify if it is a valid address
		{
			printf("Invalid address for free.\n");
			return;
		}

		node_t *miniblock_node = find_block(arena, address);
		if (address != ((miniblock_t *)miniblock_node->data)->start_address) {
			printf("Invalid address for free.\n");
			return;
		}
		// find the block that has the miniblock we have to free
		node_t *block_node = arena->alloc_list->head;
		unsigned int index1 = 0;
		while (((block_t *)block_node->data)->start_address < address &&
			   ((block_t *)block_node->data)->start_address +
					((block_t *)block_node->data)->size <
				address) {
			index1++;
			block_node = block_node->next;
		}

		if (((block_t *)block_node->data)->miniblock_list->size == 1)
		// the miniblock is the only one in the list
		// we free the miniblock and also the block
		{
			node_t *trash_node = dll_remove_nth_node(arena->alloc_list, index1);
			free(((miniblock_t *)((block_t *)trash_node->data)
					->miniblock_list->head->data)
					->rw_buffer);
			free_list(&(((block_t *)trash_node->data)->miniblock_list));
			free(trash_node->data);
			free(trash_node);
			return;
}

		// fiind the index of the miniblock in list
		unsigned int index2 = 0;
		node_t *curr = ((block_t *)block_node->data)->miniblock_list->head;
		while (((miniblock_t *)curr->data)->start_address != address) {
			index2++;
			curr = curr->next;
		}

		if (index2 == ((block_t *)block_node->data)->miniblock_list->size - 1) {
			curr = dll_remove_nth_node
			(((block_t *)block_node->data)->miniblock_list, index2);
			((block_t *)block_node->data)->size -=
			((block_t *)curr->data)->size;
			free(((miniblock_t *)curr->data)->rw_buffer);
			free(curr->data);
			free(curr);
			return;
		}

		if (index2 == 0) {
			node_t *trash_node = dll_remove_nth_node
				(((block_t *)block_node->data)->miniblock_list, index2);
			((block_t *)block_node->data)->start_address +=
				((block_t *)trash_node->data)->size;
			((block_t *)block_node->data)->size -=
				((block_t *)trash_node->data)->size;
			free(((miniblock_t *)trash_node->data)->rw_buffer);
			free(trash_node->data);
			free(trash_node);
			return;
		}

		if (index2 < ((block_t *)block_node->data)->miniblock_list->size - 1) {
			// create a new block with the miniblocks remaining after
			// the trash_node
			block_t new_block;
			new_block.start_address =
				((miniblock_t *)curr->next->data)->start_address;
			new_block.size = ((block_t *)block_node->data)->start_address +
							((block_t *)block_node->data)->size -
							((miniblock_t *)curr->next->data)->start_address;

			// create the minblock list
			new_block.miniblock_list = dll_create(sizeof(miniblock_t));
			new_block.miniblock_list->size =
				((block_t *)block_node->data)->miniblock_list->size -
							index2 - 1;

			curr = dll_remove_nth_node
				(((block_t *)block_node->data)->miniblock_list, index2);

			new_block.miniblock_list->head = curr->next;
			curr->prev->next = NULL;
			dll_add_nth_node(arena->alloc_list, index1 + 1, &new_block);
			((block_t *)block_node->data)->miniblock_list->size = index2;
			((block_t *)block_node->data)->size =
				((block_t *)curr->data)->start_address -
				((block_t *)block_node->data)->start_address;
			free(((miniblock_t *)curr->data)->rw_buffer);
			free(curr->data);
			free(curr);
		}
}

	void read(arena_t *arena, uint64_t address, uint64_t size)
{
		node_t *miniblock_node = find_block(arena, address);
		if (!miniblock_node) {
			printf("Invalid address for read.\n");
			return;
		}
		if (verif_perm_read(arena, address, size) == 0)
			return;
		uint64_t fin_address_miniblock =
			((miniblock_t *)miniblock_node->data)->size +
			((miniblock_t *)miniblock_node->data)->start_address;
		int nr_bytes = number_bytes(miniblock_node, address, size);
		if (nr_bytes < (int)size)
			printf
				("Warning: size was bigger than the block size. Reading %d "
				"characters.\n",
				nr_bytes);
		if (address + nr_bytes <= fin_address_miniblock) {
			for (int i = 0; i < (int)nr_bytes; i++)
				if (*(char *)(((miniblock_t *)miniblock_node->data)->rw_buffer +
							(address - ((miniblock_t *)miniblock_node->data)
											->start_address) +
							i) != '\0')
					printf
						("%c",
						*(char *)
						(((miniblock_t *)miniblock_node->data)->rw_buffer +
								(address - ((miniblock_t *)miniblock_node->data)
												->start_address) +
								i));
			printf("\n");
			return;
		}

		// if it extends on more miniblocks
		// fill the first miniblock
		miniblock_node = find_block(arena, address);
		int index = 0;
		for (uint64_t i = 0; i < (fin_address_miniblock - address); i++) {
			if (*(char *)(((miniblock_t *)miniblock_node->data)->rw_buffer +
						(address -
						((miniblock_t *)miniblock_node->data)->start_address) +
						i) != '\0')
				printf("%c",
					   *(char *)
					   (((miniblock_t *)miniblock_node->data)->rw_buffer +
								(address - ((miniblock_t *)miniblock_node->data)
												->start_address) +
								i));
			index++;
			nr_bytes--;
		}
		miniblock_node = miniblock_node->next;
		while (miniblock_node && nr_bytes > 0) {
			for (size_t i = 0; i < ((miniblock_t *)miniblock_node->data)->size;
				i++) {
				if (*(char *)(((miniblock_t *)miniblock_node->data)->rw_buffer +
							i) != '\0')
					printf
						("%c",
						*(char *)
						(((miniblock_t *)miniblock_node->data)->rw_buffer +
								i));
				index++;
				nr_bytes--;
			}
			miniblock_node = miniblock_node->next;
		}
		printf("\n");
}

	int verif_perm_write(arena_t *arena, const uint64_t address,
						 const uint64_t size)
{
		node_t *miniblock_node = find_block(arena, address);
		if (((miniblock_t *)miniblock_node->data)->perm != 2 &&
			((miniblock_t *)miniblock_node->data)->perm != 3 &&
			((miniblock_t *)miniblock_node->data)->perm != 6 &&
			((miniblock_t *)miniblock_node->data)->perm != 7) {
			printf("Invalid permissions for write.\n");
			return 0;
		}
		uint64_t fin_address_miniblock =
			((miniblock_t *)miniblock_node->data)->size +
			((miniblock_t *)miniblock_node->data)->start_address;
		// if the size is enough for one miniblock
		if (fin_address_miniblock - address >= size)
			return 1;

		// we need to verify the next miniblocks
		int aux = size;
		aux -= (fin_address_miniblock - address);
		miniblock_node = miniblock_node->next;
		while (miniblock_node && aux > 0) {
			if (((miniblock_t *)miniblock_node->data)->perm != 2 &&
				((miniblock_t *)miniblock_node->data)->perm != 3 &&
				((miniblock_t *)miniblock_node->data)->perm != 6 &&
				((miniblock_t *)miniblock_node->data)->perm != 7) {
				printf("Invalid permissions for write.\n");
				return 0;
			}
			aux -= ((miniblock_t *)miniblock_node->data)->size;
			miniblock_node = miniblock_node->next;
		}
		// means that we have miniblocks with permission to write
		return 1;
}

	int verif_perm_read(arena_t *arena, const uint64_t address,
						const uint64_t size)
{
		node_t *miniblock_node = find_block(arena, address);
		if (((miniblock_t *)miniblock_node->data)->perm != 4 &&
			((miniblock_t *)miniblock_node->data)->perm != 5 &&
			((miniblock_t *)miniblock_node->data)->perm != 6 &&
			((miniblock_t *)miniblock_node->data)->perm != 7) {
			printf("Invalid permissions for read.\n");
			return 0;
		}
		uint64_t fin_address_miniblock =
			((miniblock_t *)miniblock_node->data)->size +
			((miniblock_t *)miniblock_node->data)->start_address;
		// if the size is enough for one miniblock
		if (fin_address_miniblock - address >= size)
			return 1;

		// we need to verify the next miniblocks
		int aux = size;
		aux -= (fin_address_miniblock - address);
		miniblock_node = miniblock_node->next;
		while (miniblock_node && aux > 0) {
			if (((miniblock_t *)miniblock_node->data)->perm != 4 &&
				((miniblock_t *)miniblock_node->data)->perm != 5 &&
				((miniblock_t *)miniblock_node->data)->perm != 6 &&
				((miniblock_t *)miniblock_node->data)->perm != 7) {
				printf("Invalid permissions for read.\n");
				return 0;
			}
			aux -= ((miniblock_t *)miniblock_node->data)->size;
			miniblock_node = miniblock_node->next;
		}
		// means that we have miniblocks with permission to write
		return 1;
}

	int number_bytes(node_t *miniblock_node, const uint64_t address,
					 const uint64_t size)
{
		uint64_t fin_address_miniblock =
			((miniblock_t *)miniblock_node->data)->size +
			((miniblock_t *)miniblock_node->data)->start_address;
		// if the size is enough for one miniblock
		if (fin_address_miniblock - address >= size)
			return size;
		// verify how many bytes we can write/read
		int aux = size;
		aux -= (fin_address_miniblock - address);
		miniblock_node = miniblock_node->next;
		while (miniblock_node && aux > 0) {
			aux -= ((miniblock_t *)miniblock_node->data)->size;
			miniblock_node = miniblock_node->next;
		}
		if (aux > 0)
			return size - aux;
		return size;
}

	void write(arena_t *arena, const uint64_t address, const uint64_t size,
			   int8_t *data)
{
		node_t *miniblock_node = find_block(arena, address);
		if (!miniblock_node) {
			printf("Invalid address for write.\n");
			return;
		}
		if (verif_perm_write(arena, address, size) == 0)
			return;
		uint64_t fin_address_miniblock =
			((miniblock_t *)miniblock_node->data)->size +
			((miniblock_t *)miniblock_node->data)->start_address;
		int nr_bytes = number_bytes(miniblock_node, address, size);
		if (nr_bytes < (int)size)
			printf
				("Warning: size was bigger than the block size. Writing %d "
				"characters.\n",
				nr_bytes);
		if (address + nr_bytes <= fin_address_miniblock) {
			for (int i = 0; i < nr_bytes; i++)
				memcpy(((miniblock_t *)miniblock_node->data)->rw_buffer +
						(address -
						((miniblock_t *)miniblock_node->data)->start_address) +
						i,
					data + i, 1);
			return;
		}

		// if it extends on more miniblocks
		// fill the first miniblock
		miniblock_node = find_block(arena, address);
		int index = 0;
		for (uint64_t i = 0; i < (fin_address_miniblock - address); i++) {
			memcpy(((miniblock_t *)miniblock_node->data)->rw_buffer +
					(address -
						((miniblock_t *)miniblock_node->data)->start_address) +
					i,
				data + index, 1);
			index++;
			nr_bytes--;
		}
		miniblock_node = miniblock_node->next;
		while (miniblock_node && nr_bytes > 0) {
			for (size_t i = 0; i < ((miniblock_t *)miniblock_node->data)->size;
				i++) {
				memcpy(((miniblock_t *)miniblock_node->data)->rw_buffer + i,
					   data + index, 1);
				index++;
				nr_bytes--;
			}
			miniblock_node = miniblock_node->next;
		}
}

	int total_miniblocks(const arena_t *arena)
{
		node_t *block_node = arena->alloc_list->head;
		if (!arena->alloc_list)
			return 0;
		int number = 0;
		while (block_node) {
			number += ((block_t *)block_node->data)->miniblock_list->size;
			block_node = block_node->next;
		}
		return number;
}

	uint64_t used_memory(const arena_t *arena)
{
		if (arena->alloc_list->size == 0)
			return 0;
		node_t *block_node = arena->alloc_list->head;
		if (!arena->alloc_list)
			return 0;
		uint64_t memory = 0;
		while (block_node) {
			memory += ((block_t *)block_node->data)->size;
			block_node = block_node->next;
		}
		return memory;
}

	void perm_print(uint8_t perm)
{
		if (perm == 0)
			printf("---\n");
		else if (perm == 1)
			printf("--X\n");
		else if (perm == 4)
			printf("R--\n");
		else if (perm == 2)
			printf("R--\n");
		else if (perm == 6)
			printf("RW-\n");
		else if (perm == 7)
			printf("RWX\n");
		else if (perm == 5)
			printf("R-X\n");
		else if (perm == 3)
			printf("-WX\n");
}

	void pmap(const arena_t *arena)
{
		printf("Total memory: 0x%lX bytes\n", arena->arena_size);
		printf("Free memory: 0x%lX bytes\n",
			   arena->arena_size - used_memory(arena));
		printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
		if (arena->alloc_list->size == 0) {
			printf("Number of allocated miniblocks: 0\n");
			return;
		}
		printf("Number of allocated miniblocks: %d\n", total_miniblocks(arena));
		printf("\n");
		int nr_blocks = arena->alloc_list->size;

		node_t *block_node = arena->alloc_list->head;

		for (int i = 1; i < nr_blocks; i++) {
			printf("Block %d begin\n", i);
			printf("Zone: 0x%lX - 0x%lX\n",
				   ((block_t *)block_node->data)->start_address,
				   ((block_t *)block_node->data)->start_address +
				   ((block_t *)block_node->data)->size);
			node_t *miniblock_node =
				((block_t *)block_node->data)->miniblock_list->head;
			int nr_miniblocks =
				((block_t *)block_node->data)->miniblock_list->size;
			for (int j = 1; j <= nr_miniblocks; j++) {
				printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", j,
					   ((miniblock_t *)miniblock_node->data)->start_address,
					   ((miniblock_t *)miniblock_node->data)->start_address +
					   ((miniblock_t *)miniblock_node->data)->size);
				perm_print(((miniblock_t *)miniblock_node->data)->perm);
				miniblock_node = miniblock_node->next;
			}
			printf("Block %d end\n", i);
			printf("\n");
			block_node = block_node->next;
		}
		printf("Block %d begin\n", nr_blocks);
		printf("Zone: 0x%lX - 0x%lX\n",
			   ((block_t *)block_node->data)->start_address,
			   ((block_t *)block_node->data)->start_address +
			   ((block_t *)block_node->data)->size);
		node_t *miniblock_node =
			((block_t *)block_node->data)->miniblock_list->head;
		int nr_miniblocks = ((block_t *)block_node->data)->miniblock_list->size;
		for (int j = 1; j <= nr_miniblocks; j++) {
			printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", j,
				   ((miniblock_t *)miniblock_node->data)->start_address,
				   ((miniblock_t *)miniblock_node->data)->start_address +
				   ((miniblock_t *)miniblock_node->data)->size);
			perm_print(((miniblock_t *)miniblock_node->data)->perm);
			miniblock_node = miniblock_node->next;
		}
		printf("Block %d end\n", nr_blocks);
}

	void mprotect(arena_t *arena, uint64_t address, uint8_t *permission)
{
		if (!find_block(arena, address)) {
			printf("Invalid address for mprotect.\n");
			return;
		}
		node_t *miniblock = find_block(arena, address);
		if (((miniblock_t *)miniblock->data)->start_address != address) {
			printf("Invalid address for mprotect.\n");
			return;
		}
		((miniblock_t *)miniblock->data)->perm = *permission;
}
