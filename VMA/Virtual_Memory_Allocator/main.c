	#include "commands.h"
	int main(void)
{
	arena_t *arena = NULL;
	char command[64];
	while (1) {
		scanf("%s", command);
		if (strcmp(command, "ALLOC_ARENA") == 0) {
			arena = call_ALLOC_ARENA();
		} else if (strcmp(command, "DEALLOC_ARENA") == 0) {
			call_DEALLOC_ARENA(arena);
			break;
		} else if (strcmp(command, "ALLOC_BLOCK") == 0) {
			call_ALLOC_BLOCK(arena);
		} else if (strcmp(command, "FREE_BLOCK") == 0) {
			call_FREE_BLOCK(arena);
		} else if (strcmp(command, "READ") == 0) {
			call_READ(arena);
		} else if (strcmp(command, "WRITE") == 0) {
			call_WRITE(arena);
		} else if (strcmp(command, "PMAP") == 0) {
			call_PMAP(arena);
		} else if (strcmp(command, "MPROTECT") == 0) {
			call_MPROTECT(arena);
		} else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
