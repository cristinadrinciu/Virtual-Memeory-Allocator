	#include "commands.h"

	arena_t *call_ALLOC_ARENA(void)
{
		uint64_t size;
		scanf("%lu", &size);
		return alloc_arena(size);
}

	void call_DEALLOC_ARENA(arena_t *arena)
{
		dealloc_arena(arena);
}

	void call_ALLOC_BLOCK(arena_t *arena)
{
		uint64_t size;
		uint64_t address;
		scanf("%lu%lu", &address, &size);
		alloc_block(arena, address, size);
}

	void call_FREE_BLOCK(arena_t *arena)
{
		uint64_t address;
		scanf("%lu", &address);
		free_block(arena, address);
}

	void call_WRITE(arena_t *arena)
{
		uint64_t size;
		uint64_t address;
		scanf("%lu%lu", &address, &size);
		int plus_size = 10000;
		int8_t *data = malloc(size + plus_size);
		getchar();  // need a caracter after we read the command
		char *text = calloc(size + plus_size, sizeof(char));
		uint64_t read_data = 0;
		while (read_data < size) {
		// build the text, for the cases with lots of \n
			char line[256];
			if (fgets(line, 256, stdin)) {
				strcat(text, line);
				read_data += strlen(line);
			}
		}
		text[size] = '\0';
		memcpy(data, text, size);
		write(arena, address, size, data);
		free(data);
		free(text);
}

	void call_READ(arena_t *arena)
{
		uint64_t size;
		uint64_t address;
		scanf("%lu%lu", &address, &size);
		read(arena, address, size);
}

	void call_PMAP(arena_t *arena)
{
		pmap(arena);
}

	void call_MPROTECT(arena_t *arena)
{
		uint64_t address;
		uint8_t permission = 0;
		scanf("%lu", &address);
		char perm[50];
		fgets(perm, 50, stdin);
		char *ptr = strtok(perm, "\n |");
		while (ptr) {
			if (strcmp(ptr, "PROT_NONE") == 0)
				permission = 0;
			else if (strcmp(ptr, "PROT_READ") == 0)
				permission += 4;
			else if (strcmp(ptr, "PROT_WRITE") == 0)
				permission += 2;
			if (strcmp(ptr, "PROT_EXEC") == 0)
				permission += 1;
			ptr = strtok(NULL, "\n |");
		}
		mprotect(arena, address, &permission);
}
