#ifndef GAME_H_
#define GAME_H_

#include "arena.h"
#include <stdbool.h>

bool game_init(MemoryArena* game_mem);
void game_run(void);
void game_destroy(void);

#endif // !GAME_H_
