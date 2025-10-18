#ifndef GFX_H_
#define GFX_H_

#include "utils.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

extern const SDL_Color paleblue;
extern const SDL_Color paleblue_d;

bool gfx_init(const char* win_title, const u16 win_width, const u16 win_height);
SDL_Window* gfx_get_window(void);
SDL_Renderer* gfx_get_renderer(void);
void gfx_render(void);
void gfx_destroy(void);

#endif // !GFX_H_
