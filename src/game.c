#include "game.h"
#include "arena.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "level.h"
#include "player.h"
#include "state.h"
#include "utils.h"
#include <raylib.h>

static MemoryArena* game_mem;
static MemoryArena level_mem;
static GameState state;

static bool start_new(MemoryArena* level_mem);
static void update(void);
static void render(void);
static void render_debug_ui(void);

bool game_init(MemoryArena* mem)
{
    game_mem = mem;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Food Fight!");
    SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);

    if (!assetmgr_init(game_mem)) {
        util_error("Failed to init asset manager");
        return false;
    }

    Font* tf = assetmgr_load_font("assets/fonts/FiraCode-Regular.ttf", "main");
    if (!tf) {
        util_error("Failed to start level");
        return false;
    }
    Font* ef = assetmgr_load_font("assets/fonts/FontAwesome.ttf", "emoji");
    if (!ef) {
        util_error("Failed to start level");
        return false;
    }

    state.debug = true;
    state.is_running = true;

    return true;
}

void game_run(void)
{
    arena_init(&level_mem, 1 * MB); // TODO: size??

    if (!start_new(&level_mem)) {
        state.is_running = false;
    }

    while (!WindowShouldClose() && state.is_running) {
        render();
        update();
    }
}

void game_destroy(void)
{
    assetmgr_destroy();
    CloseWindow();
}

// ------------------------------------------------------------------------------------------------

static bool start_new(MemoryArena* level_mem)
{
    Vector2 screen_mid = {WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f};
    state.camera = (Camera2D){
        .offset = screen_mid,
        .zoom = SCALE,
    };

    if (!level_init(level_mem, &state) || !state.active_level->is_loaded) {
        util_error("Failed to start level");
        return false;
    }

    state.state = GAME_STATE_EDITING;

    // TODO: read level file data

    return true;
}

static void update(void)
{
    if (!state.active_level->is_loaded) {
        return;
    }

    if (input_is_key_pressed(&state.input.kb, KB_F1)) {
        state.state = state.state == GAME_STATE_EDITING ? GAME_STATE_PLAYING : GAME_STATE_EDITING;
    }
    if (input_is_key_pressed(&state.input.kb, KB_F3)) {
        state.debug = !state.debug;
    }

    Tilemap* tm = &state.active_level->tilemap;
    f32 map_w = tm->tiles_wide * tm->tile_size;
    f32 map_h = tm->tiles_high * tm->tile_size;

    if (state.input.mouse.wheel_delta != 0.0) {
        // Clamp zoom
        f32 min_zoom_x = (f32)WINDOW_WIDTH / map_w;
        f32 min_zoom_y = (f32)WINDOW_HEIGHT / map_h;
        f32 min_zoom = fmaxf(min_zoom_x, min_zoom_y);
        f32 max_zoom = MAX_ZOOM;

        state.camera.zoom = clampf(state.camera.zoom + state.input.mouse.wheel_delta, min_zoom, max_zoom);
    }

    if (input_is_mouse_down(&state.input.mouse, MB_MIDDLE)) {
        f32 m_delta_x = state.input.mouse.pos_px.x - state.input.mouse.down_pos_px.x;
        f32 m_delta_y = state.input.mouse.pos_px.y - state.input.mouse.down_pos_px.y;
        state.camera.target.x -= m_delta_x / state.camera.zoom * 0.05f;
        state.camera.target.y -= m_delta_y / state.camera.zoom * 0.05f;
    }

    // Keep map in window
    f32 half_view_w = (WINDOW_WIDTH * 0.5f) / state.camera.zoom;
    f32 half_view_h = (WINDOW_HEIGHT * 0.5f) / state.camera.zoom;

    f32 min_target_x = half_view_w;
    f32 max_target_x = map_w - half_view_w;
    f32 min_target_y = half_view_h;
    f32 max_target_y = map_h - half_view_h;

    // Clamp the target (you may already have panning logic – just apply the clamp afterwards)
    state.camera.target.x = clampf(state.camera.target.x, min_target_x, max_target_x);
    state.camera.target.y = clampf(state.camera.target.y, min_target_y, max_target_y);

    level_update();
}

static void render(void)
{
    BeginDrawing();
    {
        // GLFW shinnanigans
        input_process(&state.input);

        ClearBackground(PALEBLUE);

        BeginMode2D(state.camera);
        {
            level_render();

            if (state.state == GAME_STATE_EDITING) {
                level_render_edit_mode();
            }
        }
        EndMode2D();

        // Not affected by camera
        {
            if (state.state == GAME_STATE_EDITING) {
                level_render_edit_mode_ui();
            }
            if (state.debug) {
                render_debug_ui();
            }
        }
    }
    EndDrawing();
}

static void render_debug_ui(void)
{
    Tilemap* tm = &state.active_level->tilemap;
    u32 map_w = tm->tiles_wide * tm->tile_size;
    Font* font = assetmgr_get_font("main");

    DrawTextEx(*font,
               TextFormat("Zoom: x%.2f", state.camera.zoom),
               (Vector2){GetScreenWidth() - 150, 10},
               16,
               1.0f,
               PALEBLUE_D);
    DrawTextEx(*font,
               TextFormat("STATE: %s", state.state == GAME_STATE_PLAYING ? "playing" : "editing"),
               (Vector2){GetScreenWidth() - 150, 25},
               16,
               1.0f,
               PALEBLUE_D);

    Vector2 mpos = GetMousePosition();

    Vector2 renpos = mpos;
    if (renpos.x > GetScreenWidth() - 255) {
        renpos.x -= 280;
    }
    renpos.y = mpos.y - 55;
    if (renpos.y <= 5) {
        renpos.y += 85;
    }

    DrawTextEx(*font, TextFormat("screen_pos: %.2f x %.2f", mpos.x, mpos.y), renpos, 18, 1.0f, PALEBLUE_D);

    Vector2 wpos = screenp_to_worldp(mpos, &state.camera, GetScreenWidth(), GetScreenHeight());
    DrawTextEx(*font,
               TextFormat("world_pos: %.2f x %.2f", wpos.x, wpos.y),
               (Vector2){
                   .x = renpos.x,
                   .y = renpos.y + 20,
               },
               18,
               1.0f,
               PALEBLUE_D);

    Vector2 grid = worldp_to_gridp((Vector2){mpos.x, mpos.y}, tm->tile_size);

    DrawTextEx(*font,
               TextFormat("grid_pos: %f x %f", grid.x, grid.y),
               (Vector2){
                   .x = renpos.x,
                   .y = renpos.y + 40,
               },
               18,
               1.0f,
               PALEBLUE_D);
}
