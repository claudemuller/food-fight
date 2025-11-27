#include "edit_mode.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "raylib.h"
#include "ui.h"
#include <stdbool.h>

static GameState* state;

static void update_edit_mode(void);
static void render_edit_mode_grid(void);

static void render_edit_mode_ui(void);

static bool update_edit_mode_tileset(void);
static void render_edit_mode_tileset(void);

static void render_edit_mode_brush(void);

void edit_mode_init(GameState* game_state)
{
    state = game_state;
}

void edit_mode_update(void)
{
    if (input_is_key_pressed(&state->input.kb, KB_ESCAPE)) {
        state->state = GAME_STATE_MAIN_MENU;
    }
    if (input_is_key_pressed(&state->input.kb, KB_F1)) {
        state->state = GAME_STATE_PLAYING;
    }
    if (level_process_shared_events()) {
        return;
    }

    update_edit_mode_tileset();

    if (state->ui_hovered) {
        return;
    }

    update_edit_mode();
}

void edit_mode_render(void)
{
    BeginDrawing();
    {
        // GLFW shinnanigans
        input_process(&state->input);

        ClearBackground(PALEBLUE);

        BeginMode2D(state->camera);
        {
            render_edit_mode_grid();
            level_render();

            if (!state->ui_hovered) {
                render_edit_mode_brush();
            }
        }
        EndMode2D();

        // --- Not affected by camera -------------------------------------------------------------
        {
            render_edit_mode_ui();

            if (state->debug) {
                render_debug_ui(state);
            }
        }
    }
    EndDrawing();
}

// ································································································

static void update_edit_mode(void)
{
    static i32 scale_mode_grid_x = 0;
    static i32 scale_mode_grid_y = 0;

    Tilemap* tm = &state->active_level->tilemap;

    Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                    (u8)state->active_level->tilemap.tile_size);

    if (input_is_key_pressed(&state->input.kb, KB_F2)) {
        player_reset(&state->active_level->player);
    }

    if (input_is_key_pressed(&state->input.kb, KB_F4)) {
        if (!level_save()) {
            message_box("Error", TextFormat("Failed to save level: %s", ""));
            return;
        }

        message_box("Success!", "Level data saved successfully.");
    }

    if (input_is_key_pressed(&state->input.kb, KB_LSHFT)) {
        scale_mode_grid_x = (i32)grid.x;
        scale_mode_grid_y = (i32)grid.y;
    }
    if (input_is_key_down(&state->input.kb, KB_LSHFT)) {
        tm->brush.size.x = (f32)(abs((i32)grid.x - scale_mode_grid_x + 1) * tm->tile_size);
        tm->brush.size.x = (f32)(clamp((u32)tm->brush.size.x, tm->tile_size, MAX_BRUSH_SIZE));
        tm->brush.size.y = (f32)(abs((i32)grid.y - scale_mode_grid_y) * tm->tile_size);
        tm->brush.size.y = (f32)(clamp((u32)tm->brush.size.y, tm->tile_size, MAX_BRUSH_SIZE));
    }

    // Place tile
    if (input_is_mouse_down(&state->input.mouse, MB_LEFT)) {
        u32 start_gridx = (u32)grid.x;
        u32 start_gridy = (u32)grid.y;

        u16 brush_row_tiles = (u16)(tm->brush.size.y / tm->tile_size);
        u16 brush_col_tiles = (u16)(tm->brush.size.x / tm->tile_size);

        for (size_t i = 0; i < brush_row_tiles; ++i) {
            for (size_t j = 0; j < brush_col_tiles; ++j) {
                u32 curx = start_gridx + (u32)j;
                u32 cury = start_gridy + (u32)i;
                size_t idx = (size_t)((curx * tm->tile_size) + cury);

                tm->tiles[idx] = (Tile){
                    .src = tm->brush.src,
                    .dst =
                        {
                            .x = (f32)(curx * tm->tile_size),
                            .y = (f32)(cury * tm->tile_size),
                            .width = tm->tile_size,
                            .height = tm->tile_size,
                        },
                    .solid = true,
                };
            }
        }
    }

    // --- Delete tile ----------------------------------------------------------------------------
    if (input_is_mouse_down(&state->input.mouse, MB_RIGHT)) {
        Vector2 grid = screenp_to_gridp(state->input.mouse.pos_px, (u8)tm->tile_size);
        size_t idx = (size_t)((grid.x * tm->tile_size) + grid.y);
        tm->tiles[idx].src = (Rectangle){0};
    }
}

static void render_edit_mode_grid(void)
{
    Tilemap* tm = &state->active_level->tilemap;

    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        u32 x = i % MAP_COL_TILES;
        u32 y = (u32)(i / MAP_COL_TILES);

        DrawRectangleLinesEx(
            (Rectangle){
                .x = (f32)(x * tm->tile_size),
                .y = (f32)(y * tm->tile_size),
                .width = tm->tile_size,
                .height = tm->tile_size,
            },
            1.0f / state->camera.zoom,
            PALEBLUE_DES);
    }
}

static void render_edit_mode_ui(void)
{
    render_edit_mode_tileset();

    Tilemap* tm = &state->active_level->tilemap;

    // --- Render brush selection UI element ------------------------------------------------------
    Rectangle dst = {
        .x = (tm->tileset.pos.x + tm->tileset.size.x - tm->tileset.tile_size) * SCALE,
        .y = (tm->tileset.pos.y - tm->tileset.tile_size - 10) * SCALE,
        .width = tm->tile_size * SCALE,
        .height = tm->tile_size * SCALE,
    };

    if (tm->brush.is_set) {
        DrawTexturePro(*tm->tileset.texture, tm->brush.src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLinesEx(dst, DEBUG_UI_LINE_THICKNESS, PALEBLUE_D);
    }

    Font* font = assetmgr_get_font("main");
    DrawTextEx(*font, "Brush: ", (Vector2){dst.x - 80.0f, dst.y + 8}, UI_EDIT_MODE_SIZE, 1.0f, PALEBLUE_D);

    if (state->state == GAME_STATE_EDITING) {
        // DrawTextEx(*font, "Reset Player: F2", (Vector2){10.0f, 30}, UI_TEXT_SIZE, 1.0f, PALEBLUE_D);
        // DrawTextEx(*font, "Save Level: F4", (Vector2){10.0f, 50}, UI_TEXT_SIZE, 1.0f, PALEBLUE_D);
    }

    // --- Reset player ---------------------------------------------------------------------------
    if (ui_draw_image_button((Vector2){(f32)GetScreenWidth() - ((32.0f - UI_PADDING) * 10.0f), UI_PADDING},
                             32.0f,
                             "assets/textures/recycle-solid-full.png",
                             "Reset Player")) {
        player_reset(&state->active_level->player);
    }

    // --- Trash level ----------------------------------------------------------------------------
    if (ui_draw_image_button((Vector2){(f32)GetScreenWidth() - ((32.0f - UI_PADDING) * 8.0f), UI_PADDING},
                             32.0f,
                             "assets/textures/trash-solid-full.png",
                             "Trash level")) {
        util_debug("trash pressed");
    }

    // --- Load level -----------------------------------------------------------------------------
    if (ui_draw_image_button((Vector2){(f32)GetScreenWidth() - ((32.0f - UI_PADDING) * 6.0f), UI_PADDING},
                             32.0f,
                             "assets/textures/folder-open-solid-full.png",
                             "Load level")) {
        if (!level_load()) {
            message_box("Error", TextFormat("Failed to load level: %s", ""));
            return;
        }

        message_box("Success!", "Level data loaded successfully.");
    }

    // --- Save level -----------------------------------------------------------------------------
    if (ui_draw_image_button((Vector2){(f32)GetScreenWidth() - ((32.0f - UI_PADDING) * 4.0f), UI_PADDING},
                             32.0f,
                             "assets/textures/floppy-disk-solid-full.png",
                             "Save level")) {
        if (!level_save()) {
            message_box("Error", TextFormat("Failed to save level: %s", ""));
            return;
        }

        message_box("Success!", "Level data saved successfully.");
    }

    // --- Exit -----------------------------------------------------------------------------------
    if (ui_draw_image_button((Vector2){(f32)GetScreenWidth() - 32.0f - UI_PADDING, UI_PADDING},
                             32.0f,
                             "assets/textures/door-open-solid-full.png",
                             "Quit")) {
        // TODO: check if there are unsaved changes
        state->is_running = false;
    }
}

static bool update_edit_mode_tileset(void)
{
    Tilemap* tm = &state->active_level->tilemap;
    Tileset* ts = &tm->tileset;
    ts->active = false;

    if (!ui_is_hovering(state->input.mouse.pos_px,
                        (Rectangle){
                            .x = ts->pos.x * SCALE,
                            .y = ts->pos.y * SCALE,
                            .width = ts->size.x * SCALE,
                            .height = ts->size.y * SCALE,
                        })) {
        return false;
    }

    ts->active = true;

    f32 mouse_relx = state->input.mouse.pos_px.x - ts->pos.x * SCALE;
    f32 mouse_rely = state->input.mouse.pos_px.y - ts->pos.y * SCALE;
    ts->hovered_tile.x = floorf(mouse_relx / ts->tile_size / SCALE);
    ts->hovered_tile.y = floorf(mouse_rely / ts->tile_size / SCALE);

    // --- Set brush  -----------------------------------------------------------------------------
    if (input_is_mouse_pressed(&state->input.mouse, MB_LEFT)) {
        if (!tm->brush.is_set) {
            tm->brush.is_set = true;
        }

        if (input_is_key_down(&state->input.kb, KB_LSHFT)) {
            static i32 scale_mode_grid_x = 0;
            static i32 scale_mode_grid_y = 0;

            Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                            (u8)state->active_level->tilemap.tile_size);

            tm->brush.src.x = (f32)(abs((i32)grid.x - scale_mode_grid_x + 1) * tm->tile_size);
            tm->brush.src.x = (f32)(clamp((u32)tm->brush.size.x, tm->tile_size, MAX_BRUSH_SIZE));
            tm->brush.src.y = (f32)(abs((i32)grid.y - scale_mode_grid_y) * tm->tile_size);
            tm->brush.src.y = (f32)(clamp((u32)tm->brush.size.y, tm->tile_size, MAX_BRUSH_SIZE));
            tm->brush.src.width = tm->tileset.tile_size;
            tm->brush.src.height = tm->tileset.tile_size;
            tm->brush.size.x = tm->brush.src.width;
            tm->brush.size.y = tm->brush.src.width;
        } else {
            tm->brush.src.x = tm->tileset.hovered_tile.x * tm->tileset.tile_size;
            tm->brush.src.y = tm->tileset.hovered_tile.y * tm->tileset.tile_size;
            tm->brush.src.width = tm->tileset.tile_size;
            tm->brush.src.height = tm->tileset.tile_size;
            tm->brush.size.x = tm->tile_size;
            tm->brush.size.y = tm->tile_size;
        }
    }

    return true;
}

static void render_edit_mode_tileset(void)
{
    Tileset* ts = &state->active_level->tilemap.tileset;

    Rectangle src = {
        .x = 0,
        .y = 0,
        .width = ts->size.x,
        .height = ts->size.y,
    };
    Rectangle dst = {
        .x = ts->pos.x * SCALE,
        .y = ts->pos.y * SCALE,
        .width = ts->size.x * SCALE,
        .height = ts->size.y * SCALE,
    };

    // --- Render tilset --------------------------------------------------------------------------
    DrawTexturePro(*ts->texture, src, dst, (Vector2){0}, 0.0f, WHITE);

    // --- Render hovered tilset tile indicator ---------------------------------------------------
    if (ts->active) {
        DrawRectangleLinesEx(
            (Rectangle){
                .x = (ts->hovered_tile.x * ts->tile_size * SCALE) + (ts->pos.x * SCALE),
                .y = (ts->hovered_tile.y * ts->tile_size * SCALE) + (ts->pos.y * SCALE),
                .width = ts->tile_size * SCALE,
                .height = ts->tile_size * SCALE,
            },
            DEBUG_UI_LINE_THICKNESS,
            RED);
    }
}

// Renders the brush at the mouse pointer.
static void render_edit_mode_brush(void)
{
    Tilemap* tm = &state->active_level->tilemap;

    Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                    (u8)state->active_level->tilemap.tile_size);

    Rectangle dst = {0};

    u32 start_gridx = (u32)grid.x;
    u32 start_gridy = (u32)grid.y;

    u8 brush_col_tiles = (u8)(tm->brush.size.x / tm->tile_size);
    u8 brush_row_tiles = (u8)(tm->brush.size.y / tm->tile_size);

    for (size_t i = 0; i < brush_row_tiles; ++i) {
        for (size_t j = 0; j < brush_col_tiles; ++j) {
            u32 curx = (start_gridx + (u32)j) * tm->tile_size;
            u32 cury = (start_gridy + (u32)i) * tm->tile_size;

            dst.x = (f32)curx;
            dst.y = (f32)cury;
            dst.width = tm->tile_size;
            dst.height = tm->tile_size;

            DrawTexturePro(*tm->tileset.texture, tm->brush.src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }

    DrawRectangleLinesEx(dst, DEBUG_UI_LINE_THICKNESS / state->camera.zoom, GREEN);
}
