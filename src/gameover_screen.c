#include "gameover_screen.h"
#include "gfx.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "ui.h"
#include "utils.h"
#include <stddef.h>

static GameState* state;
static MenuItem go_mitems[GAME_OVER_N_ITEMS];
size_t n_go_mitems;

static void replay_fn(void);
static void main_menu_fn(void);
static void quit_fn(void);

void game_over_init(GameState* game_state)
{
    state = game_state;

    u16 text_height = GAME_OVER_N_ITEMS * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
    u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f + UI_HEADER_SIZE;

    go_mitems[n_go_mitems++] = create_menu_item("Replay Game", 0.0f, starty, replay_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    go_mitems[n_go_mitems++] = create_menu_item("Main Menu", 0.0f, starty, main_menu_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    go_mitems[n_go_mitems++] = create_menu_item("Quit", 0.0f, starty, quit_fn, ALIGN_CENTRE);
}

void game_over_update(void)
{
    for (size_t i = 0; i < n_go_mitems; ++i) {
        go_mitems[i].hover = false;
        if (CheckCollisionPointRec(GetMousePosition(), go_mitems[i].rec)) {
            go_mitems[i].hover = true;

            if (input_is_mouse_pressed(&state->input.mouse, MB_LEFT)) {
                if (go_mitems[i].fn) {
                    go_mitems[i].fn();
                }
            }
        }
    }
}

void game_over_render(void)
{
    BeginDrawing();
    {
        // GLFW shinnanigans
        input_process(&state->input);

        ClearBackground(PALEBLUE);

        // Not affected by camera
        {
            u16 text_height = n_go_mitems * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
            u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f;

            i32 header_size = MeasureText("Game Over", UI_HEADER_SIZE);
            u16 headerx = GetScreenWidth() * 0.5f - header_size * 0.5f;
            DrawText("Game Over", headerx, starty, UI_HEADER_SIZE, PALEBLUE_D);

            for (size_t i = 0; i < n_go_mitems; ++i) {
                Color c = PALEBLUE_D;
                if (go_mitems[i].hover) {
                    c = PALEBLUE_DES;
                    DrawText(">", go_mitems[i].rec.x - 15.0f, go_mitems[i].rec.y, go_mitems[i].rec.height, c);
                }
                DrawText(go_mitems[i].label, go_mitems[i].rec.x, go_mitems[i].rec.y, go_mitems[i].rec.height, c);
            }
        }
    }
    EndDrawing();
}

// ------------------------------------------------------------------------------------------------

static void replay_fn(void)
{
    state->state = GAME_STATE_PLAYING;
}

static void main_menu_fn(void)
{
    state->state = GAME_STATE_MAIN_MENU;
}

static void quit_fn(void)
{
    state->is_running = false;
}
