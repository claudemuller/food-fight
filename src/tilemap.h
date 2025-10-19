#ifndef TILEMAP_H_
#define TILEMAP_H_

#include "input.h"
#include "physics.h"
#include "state.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <stdbool.h>

#define MAP_TILE_SIZE 18
#define MAP_ROW_TILES (int)(WINDOW_HEIGHT / SCALE / MAP_TILE_SIZE)
#define MAP_COL_TILES (int)(WINDOW_WIDTH / SCALE / MAP_TILE_SIZE)
#define MAX_NUM_TILES (MAP_ROW_TILES * MAP_COL_TILES)

typedef struct {
    SDL_Texture* texture;
    vec2 pos;
    vec2 size;
    ivec2 hovered_tile;
    struct {
        bool is_selected;
        ivec2 tile;
    } selected;
    u8 tile_size;
    u8 pitch;
    bool inside;
} Tileset;

typedef struct {
    SDL_FRect src;
} Tile;

typedef struct {
    Tileset tileset;
    u32 width;
    u32 height;
    u32 tile_size;
    Tile* tiles;
} Tilemap;

bool tilemap_init(MemoryArena* mem, SDL_Texture* tex);
void tilemap_update(GameState* state);
void tilemap_render_tileset(void);

#endif // !TILEMAP_H_
