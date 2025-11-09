#include "game.h"
#include "arena.h"
#include "asset_manager.h"
#include "gfx.h"
#include "input.h"
#include "level.h"
#include "state.h"
#include "utils.h"
#include <raylib.h>

static MemoryArena* game_mem;
static MemoryArena level_mem;
static GameState state;
static f32 zoom;

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
        input_process(&state.input);
        update();
        render();
    }
}

void game_destroy(void)
{
    CloseWindow();
}

// ------------------------------------------------------------------------------------------------

static bool start_new(MemoryArena* level_mem)
{
    if (!level_init(level_mem, &state) || !state.active_level->is_loaded) {
        util_error("Failed to start level");
        return false;
    }

    Vector2 screen_mid = {WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f};
    f32 map_w = state.active_level->tilemap.tiles_wide * state.active_level->tilemap.tile_size;
    f32 map_h = state.active_level->tilemap.tiles_high * state.active_level->tilemap.tile_size;
    Vector2 map_centre = {map_w * 0.5f, map_h * 0.5f};

    zoom = SCALE;
    state.camera = (Camera2D){
        .target = map_centre,
        .offset = screen_mid,
        .rotation = 0.0f,
        .zoom = zoom,
    };

    state.state = GAME_STATE_EDITING;

    // state.player = (Player){
    //     // .velocity = {0, 0, 0},
    //     // .size = 1,
    //     .movement_speed = 10,
    //     // .colour = BLUE,
    // };
    // state.player.position = (Vector3){0, state.player.size + 2, 0};
    // state.player.model = LoadModelFromMesh(GenMeshCube(state.player.size, state.player.size, state.player.size));

    // g_floor = (Floor){
    //     .position = {0, 0, 0},
    //     .size = {.width = 20.0, .height = 20.0},
    //     .tint = WHITE,
    //     .texture = LoadTexture("assets/textures/ground.png"),
    // };
    // g_floor.model = LoadModelFromMesh(GenMeshPlane(g_floor.size.width, g_floor.size.height, 10, 10));
    // SetMaterialTexture(&g_floor.model.materials[0], MATERIAL_MAP_ALBEDO, g_floor.texture);

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
        zoom = state.camera.zoom;
    }

    if (input_is_mouse_down(&state.input.mouse, MB_MIDDLE)) {
        f32 m_delta_x = state.input.mouse.pos_px.x - state.input.mouse.down_pos_px.x;
        f32 m_delta_y = state.input.mouse.pos_px.y - state.input.mouse.down_pos_px.y;
        // state.camera.target.x -= m_delta_x * 0.05f;
        // state.camera.target.y -= m_delta_y * 0.05f;
        state.camera.target.x -= m_delta_x / state.camera.zoom;
        state.camera.target.y -= m_delta_y / state.camera.zoom;

        // util_info("%f %f", m_delta_x, m_delta_y);
    }

    // Keep map in window
    f32 half_view_w = (WINDOW_WIDTH * 0.5f) / state.camera.zoom;
    f32 half_view_h = (WINDOW_HEIGHT * 0.5f) / state.camera.zoom;

    f32 min_target_x = half_view_w;
    f32 max_target_x = map_w - half_view_w;
    f32 min_target_y = half_view_h;
    f32 max_target_y = map_h - half_view_h;

    // If the map is smaller than the view on a given axis, just centre it
    // if (map_w <= WINDOW_WIDTH) {
    //     min_target_x = max_target_x = map_w * 0.5f;
    // }
    // if (map_h <= WINDOW_HEIGHT) {
    //     min_target_y = max_target_y = map_h * 0.5f;
    // }

    // Clamp the target (you may already have panning logic – just apply the clamp afterwards)
    state.camera.target.x = clampf(state.camera.target.x, min_target_x, max_target_x);
    state.camera.target.y = clampf(state.camera.target.y, min_target_y, max_target_y);

    Vector2 desired_center = state.camera.target;
    // state.camera.offset.x = clamp(desired_center.x - half_view_w, 0.0f, map_w - WINDOW_WIDTH / state.camera.zoom);
    // state.camera.offset.y = clamp(desired_center.y - half_view_h, 0.0f, map_h - WINDOW_HEIGHT / state.camera.zoom);

    // util_info("%f %f", state.camera.target.x, state.camera.target.y);

    // state.camera.offset = (Vector2){
    //     WINDOW_WIDTH * 0.5f,
    //     WINDOW_HEIGHT * 0.5f,
    // };

    level_update();
}

static void render(void)
{
    BeginDrawing();
    {
        ClearBackground(PALEBLUE);

        BeginMode2D(state.camera);
        {
            level_render();

            if (state.state == GAME_STATE_EDITING) {
                level_render_edit_mode();
            }
            // brush
        }
        EndMode2D();

        // Not affected by camera
        {
            level_render_edit_mode_ui();
            render_debug_ui();
        }
    }
    EndDrawing();
}

static void render_debug_ui(void)
{
    Tilemap* tm = &state.active_level->tilemap;
    u32 map_w = tm->tiles_wide * tm->tile_size;
    DrawText(TextFormat("Zoom: x%.2f", zoom), 10, 10, 16, PALEBLUE_D);
    DrawText(
        TextFormat("STATE: %s", state.state == GAME_STATE_PLAYING ? "playing" : "editing"), 10, 25, 16, PALEBLUE_D);
    DrawText("X", map_w - 15, 10, 16, PALEBLUE_D);
}
