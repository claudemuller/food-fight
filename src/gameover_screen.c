#include "gameover_screen.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "level.h"
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
    Font* font = assetmgr_get_font("main");

    u16 text_height = GAME_OVER_N_ITEMS * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
    u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f + UI_HEADER_SIZE;

    go_mitems[n_go_mitems++] = create_menu_item("Replay Game", 0.0f, starty, font, replay_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    go_mitems[n_go_mitems++] = create_menu_item("Main Menu", 0.0f, starty, font, main_menu_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    go_mitems[n_go_mitems++] = create_menu_item("Quit", 0.0f, starty, font, quit_fn, ALIGN_CENTRE);
}

void game_over_update(void)
{
    level_process_shared_events();

    if (input_is_key_pressed(&state->input.kb, KB_ESCAPE)) {
        state->is_running = false;
    }

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
    Font* font = assetmgr_get_font("main");

    BeginDrawing();
    {
        // GLFW shinnanigans
        input_process(&state->input);

        ClearBackground(PALEBLUE);

        // Not affected by camera
        {
            u16 text_height = n_go_mitems * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
            u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f;

            Vector2 header_size = MeasureTextEx(*font, "Game Over", UI_HEADER_SIZE, 1.0f);
            u16 headerx = GetScreenWidth() * 0.5f - header_size.x * 0.5f;
            DrawTextEx(*font, "Game Over", (Vector2){headerx, starty}, UI_HEADER_SIZE, 1.0f, PALEBLUE_D);

            for (size_t i = 0; i < n_go_mitems; ++i) {
                Color c = PALEBLUE_D;
                if (go_mitems[i].hover) {
                    c = PALEBLUE_DES;
                    DrawTextEx(*font,
                               ">",
                               (Vector2){go_mitems[i].rec.x - 15.0f, go_mitems[i].rec.y},
                               go_mitems[i].rec.height,
                               1.0f,
                               c);
                }
                DrawTextEx(*font,
                           go_mitems[i].label,
                           (Vector2){go_mitems[i].rec.x, go_mitems[i].rec.y},
                           go_mitems[i].rec.height,
                           1.0f,
                           c);
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
