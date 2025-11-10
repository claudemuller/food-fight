#include "main_menu_screen.h"
#include "gfx.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "ui.h"
#include "utils.h"
#include <stddef.h>

static GameState* state;
static MenuItem mm_mitems[MAIN_MENU_N_ITEMS];
size_t n_mm_mitems;

static void start_fn(void);
static void load_level_fn(void);
static void build_level_fn(void);
static void quit_fn(void);

void main_menu_init(GameState* game_state)
{
    state = game_state;

    u16 text_height = MAIN_MENU_N_ITEMS * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
    u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f + UI_HEADER_SIZE;

    mm_mitems[n_mm_mitems++] = create_menu_item("Start game", 0.0f, starty, start_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    mm_mitems[n_mm_mitems++] = create_menu_item("Load Level", 0.0f, starty, load_level_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    mm_mitems[n_mm_mitems++] = create_menu_item("Build Level", 0.0f, starty, build_level_fn, ALIGN_CENTRE);

    starty += UI_MENU_ITEM_SIZE;
    mm_mitems[n_mm_mitems++] = create_menu_item("Quit", 0.0f, starty, quit_fn, ALIGN_CENTRE);
}

void main_menu_update(void)
{
    for (size_t i = 0; i < n_mm_mitems; ++i) {
        mm_mitems[i].hover = false;
        if (CheckCollisionPointRec(GetMousePosition(), mm_mitems[i].rec)) {
            mm_mitems[i].hover = true;

            if (input_is_mouse_pressed(&state->input.mouse, MB_LEFT)) {
                if (mm_mitems[i].fn) {
                    mm_mitems[i].fn();
                }
            }
        }
    }
}

void main_menu_render(void)
{
    BeginDrawing();
    {
        // GLFW shinnanigans
        input_process(&state->input);

        ClearBackground(PALEBLUE);

        // Not affected by camera
        {
            u16 text_height = n_mm_mitems * UI_MENU_ITEM_SIZE + UI_HEADER_SIZE;
            u16 starty = GetScreenHeight() * 0.5f - text_height * 0.5f;

            i32 header_size = MeasureText("Food Fight", UI_HEADER_SIZE);
            u16 headerx = GetScreenWidth() * 0.5f - header_size * 0.5f;
            DrawText("Food Fight", headerx, starty, UI_HEADER_SIZE, PALEBLUE_D);

            for (size_t i = 0; i < n_mm_mitems; ++i) {
                Color c = PALEBLUE_D;
                if (mm_mitems[i].hover) {
                    c = PALEBLUE_DES;
                    DrawText(">", mm_mitems[i].rec.x - 15.0f, mm_mitems[i].rec.y, mm_mitems[i].rec.height, c);
                }
                DrawText(mm_mitems[i].label, mm_mitems[i].rec.x, mm_mitems[i].rec.y, mm_mitems[i].rec.height, c);
            }
        }
    }
    EndDrawing();
}

// ------------------------------------------------------------------------------------------------

static void start_fn(void)
{
    state->state = GAME_STATE_PLAYING;
}

static void load_level_fn(void)
{
    util_debug("load level");
}

static void build_level_fn(void)
{
    state->state = GAME_STATE_EDITING;
}

static void quit_fn(void)
{
    state->is_running = false;
}
