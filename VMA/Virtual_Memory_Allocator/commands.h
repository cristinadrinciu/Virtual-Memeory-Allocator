#pragma once
#include "vma.h"

arena_t *call_ALLOC_ARENA(void);
void call_DEALLOC_ARENA(arena_t *arena);
void call_ALLOC_BLOCK(arena_t *arena);
void call_FREE_BLOCK(arena_t *arena);
void call_WRITE(arena_t *arena);
void call_READ(arena_t *arena);
void call_PMAP(arena_t *arena);
void call_MPROTECT(arena_t *arena);
