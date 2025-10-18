#include "game.h"
#include "arena.h"
#include "asset_manager.h"
#include "entity.h"
#include "gfx.h"
#include "input.h"
#include "state.h"
#include "tilemap.h"
#include "utils.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#define FPS 60
#define MILLISECS_PER_FRAME 1000 / FPS
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

static MemoryArena* game_mem;
static GameState state;
// static u32 prev_frame_ms;

static void update(void);
static void render(void);

bool game_init(MemoryArena* mem)
{
    game_mem = mem;

    if (!gfx_init("DEV Food Fight!", WINDOW_WIDTH, WINDOW_HEIGHT)) {
        util_error("Failed to init graphics");
        return false;
    }

    if (!assetmgr_init(game_mem, gfx_get_renderer())) {
        util_error("Failed to init asset manager");
        return false;
    }

    SDL_Texture* tilemap_tex = assetmgr_load_texture("assets/tilemap_packed.png");
    if (!tilemap_tex) {
        util_error("Failed to load texture");
        return false;
    }

    if (!tilemap_init(tilemap_tex)) {
        util_error("Failed to init tilemap");
        return false;
    }

    if (!entmgr_init(mem)) {
        util_error("Failed to init entity manager");
        return false;
    }

    state.state = GAME_STATE_PLAYING;
    state.is_running = true;

    return true;
}

void game_run(void)
{
    // MemoryArena frame_mem;

    while (state.is_running) {
        // arena_init(&frame_mem, sizeof(SystemCtx) * 20);
        {
            // u32 time_to_wait = MILLISECS_PER_FRAME - (SDL_GetTicks() - prev_frame_ms);
            // if (time_to_wait > 0 && time_to_wait <= MILLISECS_PER_FRAME) {
            //     SDL_Delay(time_to_wait);
            // }
            //
            // f64 dt = (SDL_GetTicks() - prev_frame_ms) / 1000.0;
            // prev_frame_ms = SDL_GetTicks();

            input_process(&state);
            update();
            render();
        }
        // arena_free(frame_mem);
    }
}

void game_destroy(void)
{
}

// ------------------------------------------------------------------------------------------------

static void update(void)
{
    if (input_keypressed(state.input.kb.escape)) {
        state.is_running = false;
    }

    if (input_keypressed(state.input.kb.f1)) {
        state.state = state.state == GAME_STATE_EDITING ? GAME_STATE_PLAYING : GAME_STATE_EDITING;
    }

    tilemap_update(&state);
}

static void render(void)
{
    SDL_Renderer* renderer = gfx_get_renderer();

    SDL_SetRenderDrawColor(renderer, paleblue.r, paleblue.g, paleblue.b, paleblue.a);
    SDL_RenderClear(renderer);

    tilemap_render_tileset();

    entity_render();

    SDL_SetRenderDrawColor(renderer, paleblue_d.r, paleblue_d.g, paleblue_d.b, paleblue_d.a);

    MouseSnapshot mouse_snapshot = input_get_mouse_snapshot();
    SDL_RenderDebugTextFormat(renderer, 10, 10, "mouse: %f x %f", mouse_snapshot.x, mouse_snapshot.y);
    SDL_RenderDebugTextFormat(renderer, 10, 20, "up: %d", state.input.kb.f1);
    SDL_RenderDebugTextFormat(renderer, 10, 30, "mleft: %d", mouse_snapshot.left.transition);

    SDL_RenderPresent(renderer);
}
