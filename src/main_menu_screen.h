#ifndef MAIN_MENU_SCREEN_H_
#define MAIN_MENU_SCREEN_H_

#include "state.h"

#define MAIN_MENU_N_ITEMS 4

void main_menu_init(GameState* game_state);
void main_menu_update(void);
void main_menu_render(void);

#endif // !MAIN_MENU_SCREEN_H_
