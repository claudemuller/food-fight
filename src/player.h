#ifndef PLAYER_H_
#define PLAYER_H_

#include "utils.h"
#include <raylib.h>

typedef struct {
    Rectangle src;
    Vector2 pos;
    Vector2 size;
    Vector2 vel;
    Texture2D* texture;
    bool on_ground;
} Player;

void player_update(const f32 dt);
void player_render(void);
void player_reset(Player* player);

#endif // !PLAYER_H_
