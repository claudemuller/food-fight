#include "game.h"
#include "arena.h"
#include "asset_manager.h"
#include "input.h"
#include "level.h"
#include "state.h"
#include "utils.h"
#include <raylib.h>

static MemoryArena* game_mem;
static MemoryArena level_mem;
static GameState state;

static bool start_new(MemoryArena* level_mem);
static void update(void);
static void render(void);

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

    state.camera = (Camera2D){
        .target = {WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f},
        .offset = {WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f},
        .rotation = 0.0f,
        .zoom = 1.0f,
    };
    state.state = GAME_STATE_EDITING;
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

    level_update();
}

static void render(void)
{
    BeginDrawing();
    {
        ClearBackground(paleblue);

        BeginMode2D(state.camera);
        {
            level_render();

            DrawText(TextFormat("STATE: %s", state.state == GAME_STATE_PLAYING ? "playing" : "editing"),
                     10,
                     10,
                     16,
                     paleblue_d);
        }
        EndMode2D();
    }
    EndDrawing();
}
