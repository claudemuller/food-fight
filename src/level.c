#include "level.h"
#include "arena.h"
#include "asset_manager.h"
#include "input.h"
#include "raylib.h"
#include "state.h"
#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static Level* active_level;
static GameState* state;

static void render_bg(void);
static void render_map(void);

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
    tm->tileset.size.x = (f32)tm->tileset.texture->width;
    tm->tileset.size.y = (f32)tm->tileset.texture->height;
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
    f32 dt = GetFrameTime();
    player_update(dt);
}

void level_render(void)
{
    render_bg();
    render_map();
    player_render();
}

bool level_load(void)
{
    // nfdu8char_t* path;
    // nfdu8filteritem_t filters[2] = {{"Level data", "bin"}};
    //
    // nfdresult_t res = NFD_OpenDialog(&path, filters, 1, "data/");
    // if (res == NFD_OKAY) {
    //     i32 bytes_read = 0;
    //     unsigned char* data = LoadFileData(path, &bytes_read);
    //     if (!data || bytes_read == 0) {
    //         util_error("Failed to load file: %s", path);
    //         return false;
    //     } else {
    //         return true;
    //     }
    //
    //     state->active_level = (Level*)data;
    //
    //     NFD_FreePathU8(path);
    //
    //     return true;
    // } else if (res == NFD_CANCEL) {
    // } else {
    //     util_error("Failed to load file: %s", NFD_GetError());
    //     return false;
    // }

    return false;
}

bool level_save(void)
{
    // nfdchar_t* path;
    // nfdfilteritem_t filters[2] = {{"Level data", "bin"}};
    //
    // nfdresult_t res = NFD_SaveDialog(&path, filters, 1, "data/", NULL);
    // if (res == NFD_OKAY) {
    //     if (!SaveFileData(path, state->active_level, sizeof(*state->active_level))) {
    //         util_error("Failed to save file: %s", path);
    //         return false;
    //     } else {
    //         return true;
    //     }
    //
    //     NFD_FreePath(path);
    //
    //     return true;
    // } else if (res == NFD_CANCEL) {
    // } else {
    //     util_error("Failed to save file: %s", NFD_GetError());
    //     return false;
    // }

    return false;
}

bool level_process_shared_events(void)
{
    if (input_is_key_pressed(&state->input.kb, KB_F3)) {
        state->debug = !state->debug;
    }
    return false;
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
    Vector2 world_space = screenp_to_worldp(p, &state->camera, (f32)GetScreenWidth(), (f32)GetScreenHeight());
    return worldp_to_gridp(world_space, tile_size);
}

// World point to grid point.
Vector2 worldp_to_gridp(const Vector2 p, const u8 tile_size)
{
    // 3️⃣ Snap to the grid
    return (Vector2){
        .x = floorf(p.x / (f32)tile_size),
        .y = floorf(p.y / (f32)tile_size),
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
