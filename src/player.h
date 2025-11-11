#ifndef PLAYER_H_
#define PLAYER_H_

#include "arena.h"
#include "state.h"
#include "utils.h"
#include <raylib.h>

#define MAX_BULLETS 50
#define BULLET_VELOCITY 100.0f

typedef enum {
    DIRECTION_LEFT = -1,
    DIRECTION_NONE = 0,
    DIRECTION_RIGHT = 1,
} Direction;

typedef struct {
    Rectangle src;
    Vector2 pos;
    Vector2 size;
    Vector2 vel;
    Texture2D* texture;
    Direction dir;
    bool on_ground;
} Player;

typedef struct {
    Vector2 pos;
    Direction dir;
} Bullet;

bool player_new(MemoryArena* level_mem, GameState* game_state);
void player_update(const f32 dt);
void player_render(void);
void player_reset(Player* player);

#endif // !PLAYER_H_
