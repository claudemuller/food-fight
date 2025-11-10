#ifndef MAIN_MENU_SCREEN_H_
#define MAIN_MENU_SCREEN_H_

#include "state.h"
#include <raylib.h>

#define MAIN_MENU_N_ITEMS 4

typedef enum {
    ALIGN_NONE,
    ALIGN_LEFT,
    ALIGN_CENTRE,
    ALIGN_RIGHT,
} TextAlignment;

typedef void (*action_fn)(void);

typedef struct {
    Rectangle rec;
    const char* label;
    action_fn fn;
    TextAlignment align;
    bool hover;
} MenuItem;

void main_menu_init(GameState* game_state);
void main_menu_update(void);
void main_menu_render(void);

#endif // !MAIN_MENU_SCREEN_H_
