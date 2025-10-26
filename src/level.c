#include "level.h"
#include "arena.h"
#include "asset_manager.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_render.h>
#include <stdbool.h>

static Level* active_level;
static GameState* state;

const Color red = {0xff, 0x00, 0x00, 0xff};
const Color paleblue = {0xd0, 0xdf, 0xff, 0xff};
const Color paleblue_d = {0x63, 0x75, 0x9e, 0xff};
const Color paleblue_des = {0xb7, 0xc2, 0xd7, 0xff};

static void render_bg(void);
static void render_map(void);
static void update_player(const f32 dt);
static void render_player(void);
static void update_edit_mode(void);
static void render_edit_mode(void);
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

    state->active_level->player = (Player){
        .pos =
            {
                .x = WINDOW_WIDTH * 0.5f,
                .y = WINDOW_HEIGHT * 0.5f,
            },
        .size = {18, 18},
    };

    // for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
    //     Tile* tile = &tm->tiles[i];
    //
    //     u32 x = i % MAP_COL_TILES;
    //     u32 y = i / MAP_COL_TILES;
    //
    //     tile->dst = (Rectangle){
    //         .x = x * MAP_TILE_SIZE,
    //         .y = y * MAP_TILE_SIZE,
    //         .width = MAP_TILE_SIZE,
    //         .height = MAP_TILE_SIZE,
    //     };
    //     tile->src = (Rectangle){
    //         .x = 0,
    //         .y = 0,
    //         .width = MAP_TILE_SIZE,
    //         .height = MAP_TILE_SIZE,
    //     };
    // }

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
    // Render positioning grid
    if (state->state == GAME_STATE_EDITING) {
        render_edit_mode();
    }

    render_bg();
    render_map();

    render_player();

    // Render editing mode UI
    if (state->state == GAME_STATE_EDITING) {
        render_edit_mode_tileset();
        if (!state->active_level->tilemap.tileset.active) {
            render_edit_mode_brush();
        }
    }
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
    player->vel.y = clamp_f(player->vel.y, -TERMINAL_VELOCITY, TERMINAL_VELOCITY);

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
}

static void render_player(void)
{
    Player player = state->active_level->player;

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

    i32 gridx = (u16)floorf(state->input.mouse.pos_px.x / tm->tile_size);
    i32 gridy = (u16)floorf(state->input.mouse.pos_px.y / tm->tile_size);

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

        u8 brush_row_tiles = tm->brush.size.y / tm->tile_size;
        u8 brush_col_tiles = tm->brush.size.x / tm->tile_size;

        for (size_t i = 0; i < brush_row_tiles; ++i) {
            for (size_t j = 0; j < brush_col_tiles; ++j) {
                u32 curx = start_gridx + j;
                u32 cury = start_gridy + i;

                tm->tiles[(curx * tm->tile_size) + cury] = (Tile){
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
}

static void render_edit_mode(void)
{
    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        u32 x = i % MAP_COL_TILES;
        u32 y = i / MAP_COL_TILES;

        DrawRectangleLinesEx(
            (Rectangle){
                .x = x * MAP_TILE_SIZE,
                .y = y * MAP_TILE_SIZE,
                .width = MAP_TILE_SIZE,
                .height = MAP_TILE_SIZE,
            },
            1.0f,
            paleblue_des);
    }

    // Render brush selection UI element
    Tilemap* tm = &active_level->tilemap;

    Rectangle dst = {
        .x = tm->tileset.pos.x + tm->tileset.size.x - tm->tileset.tile_size,
        .y = tm->tileset.pos.y - tm->tileset.tile_size - 10,
        .width = tm->tile_size,
        .height = tm->tile_size,
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
                                    .x = ts->pos.x,
                                    .y = ts->pos.y,
                                    .width = ts->size.x,
                                    .height = ts->size.y,
                                })) {
        return false;
    }

    ts->active = true;

    f32 mouse_relx = state->input.mouse.pos_px.x - ts->pos.x;
    f32 mouse_rely = state->input.mouse.pos_px.y - ts->pos.y;
    ts->hovered_tile.x = (u32)floorf(mouse_relx / ts->tile_size);
    ts->hovered_tile.y = (u32)floorf(mouse_rely / ts->tile_size);

    if (input_is_mouse_pressed(&state->input.mouse, MB_LEFT)) {
        if (!tm->brush.is_set) {
            tm->brush.is_set = true;
        }
        tm->brush.src.x = tm->tileset.hovered_tile.x * tm->tileset.tile_size;
        tm->brush.src.y = tm->tileset.hovered_tile.y * tm->tileset.tile_size;
        tm->brush.src.width = tm->tileset.tile_size;
        tm->brush.src.height = tm->tileset.tile_size;
        tm->brush.size.x = tm->tile_size;
        tm->brush.size.y = tm->tile_size;
    }

    return true;
}

static void render_edit_mode_tileset(void)
{
    Tileset* ts = &active_level->tilemap.tileset;

    DrawTexture(*ts->texture, ts->pos.x, ts->pos.y, WHITE);

    DrawRectangleLinesEx(
        (Rectangle){
            .x = ts->hovered_tile.x * ts->tile_size + ts->pos.x,
            .y = ts->hovered_tile.y * ts->tile_size + ts->pos.y,
            .width = ts->tile_size,
            .height = ts->tile_size,
        },
        1.0f,
        RED);
}

static void render_edit_mode_brush(void)
{
    Tilemap* tm = &active_level->tilemap;

    u32 gridx = (u16)floorf(state->input.mouse.pos_px.x / tm->tile_size);
    u32 gridy = (u16)floorf(state->input.mouse.pos_px.y / tm->tile_size);

    Rectangle dst = (Rectangle){
        .width = tm->tile_size,
        .height = tm->tile_size,
    };
    u32 x = gridx * tm->tile_size;
    u32 y = gridy * tm->tile_size;

    u8 brush_row_tiles = tm->brush.size.y / tm->tile_size;
    u8 brush_col_tiles = tm->brush.size.x / tm->tile_size;

    for (size_t i = 0; i < brush_row_tiles; ++i) {
        for (size_t j = 0; j < brush_col_tiles; ++j) {
            dst.x = x + (j * tm->tile_size);
            dst.y = y + (i * tm->tile_size);

            DrawTexturePro(*tm->tileset.texture, tm->brush.src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
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
