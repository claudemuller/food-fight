#include "gfx.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_pixels.h>

static SDL_Window* window;
static SDL_Renderer* renderer;

const SDL_Color paleblue = {0xd0, 0xdf, 0xff, 0xff};
const SDL_Color paleblue_d = {0x63, 0x75, 0x9e, 0xff};

bool gfx_init(const char* win_title, const u16 win_width, const u16 win_height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        util_error("Failed to init SDL: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(win_title, win_width, win_height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        util_error("Failed to create window: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        util_error("Failed to create renderer: %s", SDL_GetError());
        return false;
    }
    SDL_SetRenderLogicalPresentation(
        renderer, win_width / SCALE, win_height / SCALE, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return true;
}

SDL_Window* gfx_get_window(void)
{
    return window;
}

SDL_Renderer* gfx_get_renderer(void)
{
    return renderer;
}

void gfx_render(void)
{
    // SDL_SetRenderDrawColor(renderer, 0xb5, 0xc7, 0xeb, 0xff);
    // SDL_RenderClear(renderer);
    //
    // SDL_SetRenderDrawColor(renderer, 0x60, 0x69, 0x7c, 0xff);
    // SDL_RenderDebugText(renderer, 10, 10, "test");
    //
    // SDL_RenderPresent(renderer);
}

void gfx_destroy(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
