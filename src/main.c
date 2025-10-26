#include "arena.h"
#include "game.h"
#include <stdlib.h>

int main(void)
{
    MemoryArena game_mem;
    arena_init(&game_mem, 1 * MB);

    if (!game_init(&game_mem)) {
        util_fatal("Failed to init game.");
    }

    game_run();

    game_destroy();
    arena_free(&game_mem);

    return EXIT_SUCCESS;
}
