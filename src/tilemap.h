#ifndef TILEMAP_H_
#define TILEMAP_H_

#include "input.h"
#include "physics.h"
#include "state.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Texture* texture;
    vec2 pos;
    vec2 size;
    ivec2 hovered_tile;
    struct {
        bool selected;
        ivec2 tile;
    } selected;
    u8 tile_size;
    u8 pitch;
    bool inside;
} Tileset;

typedef struct {
    Tileset tileset;
    u32 width;
    u32 height;
    u32 tile_size;
} Tilemap;

bool tilemap_init(SDL_Texture* tex);
void tilemap_update(GameState* state);
void tilemap_render_tileset(void);

#endif // !TILEMAP_H_
