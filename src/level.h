#ifndef LEVEL_H_
#define LEVEL_H_

#include "arena.h"
#include "raylib.h"
#include "state.h"
#include "utils.h"
#include <stdbool.h>

#define SCALE 2.0f
#define MAP_TILE_SIZE 18
#define MAP_COL_TILES 80
#define MAP_ROW_TILES 50
#define MAX_NUM_TILES (MAP_ROW_TILES * MAP_COL_TILES)

#define UI_DEBUG_FONT_SIZE 16
#define MAX_BRUSH_SIZE (MAP_TILE_SIZE * 20)

extern const Color red;
extern const Color paleblue;
extern const Color paleblue_d;
extern const Color paleblue_des;

typedef struct {
    Rectangle src;
    Rectangle dst;
    bool solid;
} Tile;

typedef struct {
    Vector2 pos;
    Vector2 size;
    Rectangle src;
    bool is_set;
} Brush;

typedef struct {
    Texture2D* texture;
    Vector2 hovered_tile;
    Vector2 pos;
    Vector2 size;
    u16 tile_size;
    bool active;
} Tileset;

typedef struct {
    u16 tiles_wide;
    u16 tiles_high;
    u16 tile_size;
    Brush brush;
    Tileset tileset;
    Tile* tiles;
} Tilemap;

typedef struct {
    Rectangle src;
    Vector2 pos;
    Vector2 size;
    Vector2 vel;
    Texture2D* texture;
    bool on_ground;
} Player;

typedef struct Level {
    Texture2D* bg_texture;
    Tilemap tilemap;
    Player player;
    // colliders;
    bool is_loaded;
} Level;

bool level_init(MemoryArena* level_mem, GameState* state);
void level_update(void);
void level_render(void);
void level_render_edit_mode(void);

#endif // !LEVEL_H_
