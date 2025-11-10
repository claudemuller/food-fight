#include "level.h"
#include "arena.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_render.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static Level* active_level;
static GameState* state;

static void render_bg(void);
static void render_map(void);
static void update_player(const f32 dt);
static void render_player(void);
static void update_edit_mode(void);
static void render_edit_mode_grid(void);
static void render_edit_mode_ui(void);
static bool update_edit_mode_tileset(void);
static void render_edit_mode_tileset(void);
static void render_edit_mode_brush(void);
static inline void get_overlapping_tiles(Rectangle r, size_t* out_first, size_t* out_last);

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

    Vector2 player_wpos = screenp_to_worldp(
        (Vector2){
            .x = GetScreenWidth() * 0.5f,
            .y = GetScreenHeight() * 0.5f,
        },
        &state->camera,
        GetScreenWidth(),
        GetScreenHeight());

    state->active_level->player = (Player){
        .pos = player_wpos,
        .size =
            {
                .x = 18, // * state->camera.zoom,
                .y = 18, // * state->camera.zoom,
            },
    };
    util_debug("%f %f", state->active_level->player.pos.x, state->active_level->player.pos.y);

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

    update_player(dt);
}

void level_render(void)
{
    render_bg();

    // Render positioning grid
    if (state->state == GAME_STATE_EDITING) {
        render_edit_mode_grid();
    }

    render_map();
    render_player();
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
    f32 view_w = screen_w / cam->zoom;
    f32 view_h = screen_h / cam->zoom;

    // Get camera bounds
    f32 camera_left = cam->target.x - view_w * 0.5f;
    f32 camera_top = cam->target.y - view_h * 0.5f;

    // Convert screen space point to world space point
    f32 world_x = camera_left + (spos.x / screen_w) * view_w;
    f32 world_y = camera_top + (spos.y / screen_h) * view_h;

    util_debug("spos: %f %f - czoom: %f - view: %f %f - cpos: %f %f - wpos: %f %f",
               screen_w,
               screen_h,
               cam->zoom,
               view_w,
               view_h,
               camera_left,
               camera_top,
               world_x,
               world_y);

    return (Vector2){
        .x = world_x,
        .y = world_y,
    };
}

// World point to grid point.
u32 worldp_to_gridp(float mouse_x, float mouse_y, u8 tile_size)
{
    Vector2 world_space =
        screenp_to_worldp((Vector2){mouse_x, mouse_y}, &state->camera, GetScreenWidth(), GetScreenHeight());

    // 3️⃣ Snap to the grid
    int gridx = (int)floorf(world_space.x / (float)tile_size);
    int gridy = (int)floorf(world_space.y / (float)tile_size);

    return (gridx << 16) | (gridy & 0xFFFF);
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

static void update_player(float dt)
{
    Player* player = &state->active_level->player;
    Tilemap* tm = &state->active_level->tilemap;

    if (input_is_key_down(&state->input.kb, KB_A)) {
        player->vel.x = -PLAYER_SPEED;
    } else if (input_is_key_down(&state->input.kb, KB_D)) {
        player->vel.x = PLAYER_SPEED;
    } else {
        player->vel.x = 0.0f;
    }

    player->vel.y += GRAVITY * dt;
    player->vel.y = clampf(player->vel.y, -TERMINAL_VELOCITY, TERMINAL_VELOCITY);

    f32 new_x = player->pos.x + player->vel.x * dt;
    f32 new_y = player->pos.y + player->vel.y * dt;

    // Horizontal sweep and resolution
    if (player->vel.x != 0.0f) {
        Rectangle horz_box = {
            .x = (player->vel.x > 0) ? player->pos.x + player->size.x : new_x,
            .y = player->pos.y,
            .width = fabsf(player->vel.x * dt),
            .height = player->size.y,
        };

        size_t first, last;
        get_overlapping_tiles(horz_box, &first, &last);

        // util_debug("first: %zu, last:%zu", first, last);

        // TODO: fix the first and last not being correct
        // for (size_t i = first; i <= last; ++i) {
        for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
            Tile* tile = &tm->tiles[i];
            if (!tile->solid) {
                continue;
            }

            if (CheckCollisionRecs(horz_box, tile->dst)) {
                util_debug("collisios");

                if (player->vel.x > 0) {
                    // Resolve to the right
                    new_x = tile->dst.x - player->size.x - 0.001f;
                } else {
                    // Resolve to the left
                    new_x = tile->dst.x + tile->dst.width + 0.001f;
                }
                player->vel.x = 0.0f;
                break;
            }
        }
    }

    // Vertical sweep and resolution
    if (player->vel.y != 0.0f) {
        Rectangle vert_box = {
            .x = new_x,
            .y = (player->vel.y > 0) ? player->pos.y + player->size.y : new_y,
            .width = player->size.x,
            .height = fabsf(player->vel.y * dt),
        };

        size_t first, last;
        get_overlapping_tiles(vert_box, &first, &last);

        player->on_ground = false;

        // TODO: fix the first and last not being correct
        // for (size_t i = first; i <= last; ++i) {
        for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
            Tile* tile = &tm->tiles[i];
            if (!tile->solid) continue;

            if (CheckCollisionRecs(vert_box, tile->dst)) {
                if (player->vel.y > 0) {
                    // Player is falling
                    new_y = tile->dst.y - player->size.y - 0.001f;
                    player->on_ground = true;
                } else {
                    // Player is moving up/jumping
                    new_y = tile->dst.y + tile->dst.height + 0.001f;
                }
                player->vel.y = 0.0f;
                break;
            }
        }
    }

    player->pos.x = new_x;
    player->pos.y = new_y;
    state->camera.target = player->pos;
}

static void render_player(void)
{
    Player player = state->active_level->player;

    // Scale added for size in player creation and in update for pos
    DrawRectangleRec(
        (Rectangle){
            .x = player.pos.x,
            .y = player.pos.y,
            .width = player.size.x,
            .height = player.size.y,
        },
        GREEN);

    // --------------------------------------------------------------------------------------------
    // TODO: debug overlap
    //
    f32 dt = GetFrameTime();
    f32 new_x = player.pos.x + player.vel.x * dt;
    Rectangle horz_box = {
        .x = (player.vel.x > 0) ? player.pos.x + player.size.x : new_x,
        .y = player.pos.y,
        .width = fabsf(player.vel.x * dt),
        .height = player.size.y,
    };
    DrawRectangleRec(horz_box, RED);

    size_t first, last;
    get_overlapping_tiles(horz_box, &first, &last);

    // for (size_t i = first; i <= last; ++i) {
    //     u32 x = i % MAP_COL_TILES;
    //     u32 y = i / MAP_COL_TILES;
    //
    //     DrawRectangleRec(
    //         (Rectangle){
    //             .x = x * MAP_TILE_SIZE,
    //             .y = y * MAP_TILE_SIZE,
    //             .width = MAP_TILE_SIZE,
    //             .height = MAP_TILE_SIZE,
    //         },
    //         RED);
    // }
}

static void update_edit_mode(void)
{
    static i32 mouse_down_grid_x = 0;
    static i32 mouse_down_grid_y = 0;

    Tilemap* tm = &active_level->tilemap;

    u32 packed_coords = worldp_to_gridp(
        state->input.mouse.pos_px.x, state->input.mouse.pos_px.y, state->active_level->tilemap.tile_size);
    u16 gridx = (packed_coords >> 16) & 0xFFFF; // high 16 bits
    u16 gridy = packed_coords & 0xFFFF;

    if (input_is_key_pressed(&state->input.kb, KB_LSHFT)) {
        mouse_down_grid_x = gridx;
        mouse_down_grid_y = gridy;
    }
    if (input_is_key_down(&state->input.kb, KB_LSHFT)) {
        tm->brush.size.x = abs(gridx - mouse_down_grid_x) * tm->tile_size;
        tm->brush.size.x = clamp(tm->brush.size.x, tm->tile_size, MAX_BRUSH_SIZE);
        tm->brush.size.y = abs(gridy - mouse_down_grid_y) * tm->tile_size;
        tm->brush.size.y = clamp(tm->brush.size.y, tm->tile_size, MAX_BRUSH_SIZE);
    }

    // Place tile
    if (input_is_mouse_down(&state->input.mouse, MB_LEFT)) {
        u32 start_gridx = gridx;
        u32 start_gridy = gridy;

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

                util_debug("i:%f %f %f",
                           idx,
                           tm->tiles[(curx * tm->tile_size) + cury].dst.x,
                           tm->tiles[(curx * tm->tile_size) + cury].dst.y);
            }
        }
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
    DrawTexturePro(*tm->tileset.texture, tm->brush.src, dst, (Vector2){0, 0}, 0.0f, WHITE);
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
        tm->brush.src.x = tm->tileset.hovered_tile.x * tm->tileset.tile_size;
        tm->brush.src.y = tm->tileset.hovered_tile.y * tm->tileset.tile_size;
        tm->brush.src.width = tm->tileset.tile_size;
        tm->brush.src.height = tm->tileset.tile_size;
        // TODO: right?
        tm->brush.size.x = tm->tile_size;
        tm->brush.size.y = tm->tile_size;
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

    u32 packed_coords = worldp_to_gridp(
        state->input.mouse.pos_px.x, state->input.mouse.pos_px.y, state->active_level->tilemap.tile_size);
    u16 gridx = (packed_coords >> 16) & 0xFFFF; // high 16 bits
    u16 gridy = packed_coords & 0xFFFF;

    Rectangle dst = {0};

    u32 start_gridx = gridx;
    u32 start_gridy = gridy;

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

static inline void get_overlapping_tiles(Rectangle r, size_t* out_first, size_t* out_last)
{
    // Clamp to map bounds
    int col_start = (int)floorf(r.x / MAP_TILE_SIZE);
    int row_start = (int)floorf(r.y / MAP_TILE_SIZE);
    int col_end = (int)ceilf((r.x + r.width) / MAP_TILE_SIZE);
    int row_end = (int)ceilf((r.y + r.height) / MAP_TILE_SIZE);

    // Clamp to legal indices
    if (col_start < 0) col_start = 0;
    if (row_start < 0) row_start = 0;
    if (col_end > MAP_COL_TILES) col_end = MAP_COL_TILES;
    if (row_end > MAP_ROW_TILES) row_end = MAP_ROW_TILES;

    // Convert the 2‑D region to a 1‑D range (row‑major)
    *out_first = (size_t)(row_start * MAP_COL_TILES + col_start);
    *out_last = (size_t)((row_end * MAP_COL_TILES + col_end) - 1);
}
