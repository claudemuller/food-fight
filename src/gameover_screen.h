#ifndef GAME_OVER_SCREEN_H_
#define GAME_OVER_SCREEN_H_

#include "state.h"

#define GAME_OVER_N_ITEMS 3

void game_over_init(GameState* game_state);
void game_over_update(void);
void game_over_render(void);

#endif // !GAME_OVER_SCREEN_H_
