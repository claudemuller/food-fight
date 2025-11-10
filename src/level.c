#include "level.h"
#include "arena.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "ui.h"
#include "utils.h"
#include <SDL3/SDL_render.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static Level* active_level;
static GameState* state;

static void render_bg(void);
static void render_map(void);

static void update_edit_mode(void);
static void render_edit_mode_grid(void);
static void render_edit_mode_ui(void);
static bool update_edit_mode_tileset(void);
static void render_edit_mode_tileset(void);
static void render_edit_mode_brush(void);

bool level_init(MemoryArena* level_mem, GameState* game_state)
{
    state = game_state;

    active_level = (Level*)arena_alloc_aligned(level_mem, sizeof(Level), 16);
    if (!active_level) {
        util_error("Failed to init level mem");
        return false;
    }
    state->active_level = active_level;

    Tilemap* tm = &active_level->tilemap;
    tm->tile_size = MAP_TILE_SIZE;
    tm->tiles_wide = MAP_COL_TILES;
    tm->tiles_high = MAP_ROW_TILES;

    tm->tileset.texture = assetmgr_load_texture("assets/textures/tilemap.png");
    if (!tm->tileset.texture) {
        util_error("Failed to load tilemap texture");
        return false;
    }
    tm->tileset.tile_size = MAP_TILE_SIZE;
    tm->tileset.size.x = tm->tileset.texture->width;
    tm->tileset.size.y = tm->tileset.texture->height;
    tm->tileset.pos.x = (f32)(WINDOW_WIDTH / SCALE) - tm->tileset.size.x - 10;
    tm->tileset.pos.y = (f32)(WINDOW_HEIGHT / SCALE) - tm->tileset.size.y - 10;

    tm->tiles = (Tile*)arena_alloc_aligned(level_mem, sizeof(Tile) * MAX_NUM_TILES, 16);
    if (!tm->tiles) {
        util_error("Failed to allocate for map tiles");
        return false;
    }

    f32 map_w = state->active_level->tilemap.tiles_wide * state->active_level->tilemap.tile_size;
    f32 map_h = state->active_level->tilemap.tiles_high * state->active_level->tilemap.tile_size;
    Vector2 map_centre = {map_w * 0.5f, map_h * 0.5f};
    state->camera.target = map_centre;

    if (!player_new(level_mem, state)) {
        util_error("Failed to start level");
        return false;
    }

    player_reset(&state->active_level->player);
    state->camera.target = state->active_level->player.pos;

    active_level->is_loaded = true;

    return true;
}

void level_update(void)
{
    // Update editing mode UI
    if (state->state == GAME_STATE_EDITING) {
        if (update_edit_mode_tileset()) return;

        update_edit_mode();

        return;
    }

    f32 dt = GetFrameTime();

    player_update(dt);
}

void level_render(void)
{
    render_bg();

    // Render positioning grid
    if (state->state == GAME_STATE_EDITING) {
        render_edit_mode_grid();
    }

    render_map();
    player_render();
}

void level_render_edit_mode(void)
{
    if (!state->active_level->tilemap.tileset.active) {
        render_edit_mode_brush();
    }
}

void level_render_edit_mode_ui(void)
{
    render_edit_mode_ui();
}

// Converts a 2D screen position (spos) to a 2D world position.
Vector2 screenp_to_worldp(const Vector2 spos, Camera2D* cam, const f32 screen_w, const f32 screen_h)
{
    // Divide by zoom to get number of world units that fits on screen
    f32 view_w = screen_w;
    f32 view_h = screen_h;
    if (cam->zoom > 0.0f || cam->zoom < 0.0f) {
        view_w = screen_w / cam->zoom;
        view_h = screen_h / cam->zoom;
    }

    // Get camera bounds
    f32 camera_left = cam->target.x - view_w * 0.5f;
    f32 camera_top = cam->target.y - view_h * 0.5f;

    // Convert screen space point to world space point
    f32 world_x = camera_left + (spos.x / screen_w) * view_w;
    f32 world_y = camera_top + (spos.y / screen_h) * view_h;

    return (Vector2){
        .x = world_x,
        .y = world_y,
    };
}

// Screen point to grid point.
Vector2 screenp_to_gridp(const Vector2 p, const u8 tile_size)
{
    Vector2 world_space = screenp_to_worldp(p, &state->camera, GetScreenWidth(), GetScreenHeight());

    return worldp_to_gridp(world_space, tile_size);
}

// World point to grid point.
Vector2 worldp_to_gridp(const Vector2 p, const u8 tile_size)
{
    // 3️⃣ Snap to the grid
    return (Vector2){
        .x = (int)floorf(p.x / (float)tile_size),
        .y = (int)floorf(p.y / (float)tile_size),
    };
}

// ------------------------------------------------------------------------------------------------

static void render_bg(void)
{
}

static void render_map(void)
{
    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        Tile tile = active_level->tilemap.tiles[i];

        DrawTexturePro(*active_level->tilemap.tileset.texture, tile.src, tile.dst, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

static void update_edit_mode(void)
{
    static i32 scale_mode_grid_x = 0;
    static i32 scale_mode_grid_y = 0;

    Tilemap* tm = &active_level->tilemap;

    Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                    state->active_level->tilemap.tile_size);

    if (input_is_key_pressed(&state->input.kb, KB_F2)) {
        player_reset(&state->active_level->player);
    }

    if (input_is_key_pressed(&state->input.kb, KB_LSHFT)) {
        scale_mode_grid_x = grid.x;
        scale_mode_grid_y = grid.y;
    }
    if (input_is_key_down(&state->input.kb, KB_LSHFT)) {
        tm->brush.size.x = abs((i32)grid.x - scale_mode_grid_x + 1) * tm->tile_size;
        tm->brush.size.x = clamp(tm->brush.size.x, tm->tile_size, MAX_BRUSH_SIZE);
        tm->brush.size.y = abs((i32)grid.y - scale_mode_grid_y) * tm->tile_size;
        tm->brush.size.y = clamp(tm->brush.size.y, tm->tile_size, MAX_BRUSH_SIZE);
    }

    // Place tile
    if (input_is_mouse_down(&state->input.mouse, MB_LEFT)) {
        u32 start_gridx = grid.x;
        u32 start_gridy = grid.y;

        u16 brush_row_tiles = (u16)(tm->brush.size.y / tm->tile_size);
        u16 brush_col_tiles = (u16)(tm->brush.size.x / tm->tile_size);

        for (size_t i = 0; i < brush_row_tiles; ++i) {
            for (size_t j = 0; j < brush_col_tiles; ++j) {
                u32 curx = start_gridx + j;
                u32 cury = start_gridy + i;
                size_t idx = (size_t)((curx * tm->tile_size) + cury);

                tm->tiles[idx] = (Tile){
                    .src = tm->brush.src,
                    .dst =
                        {
                            .x = curx * tm->tile_size,
                            .y = cury * tm->tile_size,
                            .width = tm->tile_size,
                            .height = tm->tile_size,
                        },
                    .solid = true,
                };
            }
        }
    }

    // Delete tile
    if (input_is_mouse_down(&state->input.mouse, MB_RIGHT)) {
        Vector2 grid = screenp_to_gridp(state->input.mouse.pos_px, tm->tile_size);
        size_t idx = (size_t)((grid.x * tm->tile_size) + grid.y);
        tm->tiles[idx].src = (Rectangle){0};
    }
}

static void render_edit_mode_grid(void)
{
    Tilemap* tm = &state->active_level->tilemap;

    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        u32 x = i % MAP_COL_TILES;
        u32 y = i / MAP_COL_TILES;

        DrawRectangleLinesEx(
            (Rectangle){
                .x = x * tm->tile_size,
                .y = y * tm->tile_size,
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

    // Render brush selection UI element
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
        DrawTextEx(*font, "Editing:", (Vector2){10.0f, 10}, UI_HEADER_SIZE_ALT, 1.0f, PALEBLUE_D);
        DrawTextEx(*font, "Reset Player: F2", (Vector2){10.0f, 30}, UI_TEXT_SIZE, 1.0f, PALEBLUE_D);
    }
}

static bool update_edit_mode_tileset(void)
{
    Tilemap* tm = &active_level->tilemap;
    Tileset* ts = &tm->tileset;
    ts->active = false;

    if (!CheckCollisionPointRec(state->input.mouse.pos_px,
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
    ts->hovered_tile.x = (u32)floorf(mouse_relx / ts->tile_size / SCALE);
    ts->hovered_tile.y = (u32)floorf(mouse_rely / ts->tile_size / SCALE);

    if (input_is_mouse_pressed(&state->input.mouse, MB_LEFT)) {
        if (!tm->brush.is_set) {
            tm->brush.is_set = true;
        }

        if (input_is_key_down(&state->input.kb, KB_LSHFT)) {
            static i32 scale_mode_grid_x = 0;
            static i32 scale_mode_grid_y = 0;

            Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                            state->active_level->tilemap.tile_size);

            tm->brush.src.x = abs((i32)grid.x - scale_mode_grid_x + 1) * tm->tile_size;
            tm->brush.src.x = clamp(tm->brush.size.x, tm->tile_size, MAX_BRUSH_SIZE);
            tm->brush.src.y = abs((i32)grid.y - scale_mode_grid_y) * tm->tile_size;
            tm->brush.src.y = clamp(tm->brush.size.y, tm->tile_size, MAX_BRUSH_SIZE);
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
    Tileset* ts = &active_level->tilemap.tileset;

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

    // Render tilset
    DrawTexturePro(*ts->texture, src, dst, (Vector2){0}, 0.0f, WHITE);

    // Render hovered tilset tile indicator
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
    Tilemap* tm = &active_level->tilemap;

    Vector2 grid = screenp_to_gridp((Vector2){state->input.mouse.pos_px.x, state->input.mouse.pos_px.y},
                                    state->active_level->tilemap.tile_size);

    Rectangle dst = {0};

    u32 start_gridx = grid.x;
    u32 start_gridy = grid.y;

    u8 brush_col_tiles = tm->brush.size.x / tm->tile_size;
    u8 brush_row_tiles = tm->brush.size.y / tm->tile_size;

    for (size_t i = 0; i < brush_row_tiles; ++i) {
        for (size_t j = 0; j < brush_col_tiles; ++j) {
            u32 curx = (start_gridx + j) * tm->tile_size;
            u32 cury = (start_gridy + i) * tm->tile_size;

            dst.x = curx;
            dst.y = cury;
            dst.width = tm->tile_size;
            dst.height = tm->tile_size;

            DrawTexturePro(*tm->tileset.texture, tm->brush.src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }

    DrawRectangleLinesEx(dst, DEBUG_UI_LINE_THICKNESS / state->camera.zoom, GREEN);
}
