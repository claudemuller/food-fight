#ifndef GAME_H_
#define GAME_H_

#include "arena.h"
#include <stdbool.h>

#define FPS 60
#define MILLISECS_PER_FRAME 1000 / FPS

bool game_init(MemoryArena* game_mem);
void game_run(void);
void game_destroy(void);

#endif // !GAME_H_
