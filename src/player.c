#include "player.h"
#include "gfx.h"
#include "level.h"
#include "raylib.h"

static GameState* state;
static Player* player;
static Bullet bullets[MAX_BULLETS];
static size_t n_bullets;
static size_t live_bullets[MAX_BULLETS];
static size_t n_live_bullets;

static inline void get_overlapping_tiles(Rectangle r, size_t* out_first, size_t* out_last);

bool player_new(MemoryArena* level_mem, GameState* game_state)
{
    state = game_state;

    player = (Player*)arena_alloc_aligned(level_mem, sizeof(Player), 16);
    if (!player) {
        util_error("Failed to create player");
        return false;
    }

    player_reset(player);
    state->camera.target = player->pos;

    return true;
}

void player_update(float dt)
{
    Tilemap* tm = &state->active_level->tilemap;

    // TODO: should be reset for animation/movement frames but not for bullet initial dir
    // player->dir = DIRECTION_NONE;

    if (input_is_key_down(&state->input.kb, KB_A) || input_gamepad_button_down(3, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
        player->vel.x = -PLAYER_SPEED;
        player->dir = DIRECTION_LEFT;
    } else if (input_is_key_down(&state->input.kb, KB_D) ||
               input_gamepad_button_down(3, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
        player->vel.x = PLAYER_SPEED;
        player->dir = DIRECTION_RIGHT;
    } else {
        player->vel.x = 0.0f;
    }

    if (input_is_key_pressed(&state->input.kb, KB_W) ||
        input_gamepad_button_pressed(3, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        if (player->on_ground) {
            player->vel.y -= PLAYER_JUMP_STRENGTH;
            player->on_ground = false;
        }
    }

    if (input_is_key_pressed(&state->input.kb, KB_SPACE) ||
        input_gamepad_button_pressed(3, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        bullets[n_bullets] = (Bullet){
            .pos = player->pos,
            .dir = player->dir,
        };
        live_bullets[n_live_bullets++] = n_bullets;
        n_bullets++;
    }

    for (size_t i = 0; i < n_live_bullets; ++i) {
        Bullet* b = &bullets[live_bullets[i]];
        b->pos.x += BULLET_VELOCITY * (f32)b->dir * dt;

        // TODO: bullet collision with blocks and off screen
    }

    player->vel.y += GRAVITY * dt;
    player->vel.y = clampf(player->vel.y, -TERMINAL_VELOCITY, TERMINAL_VELOCITY);

    f32 new_x = player->pos.x + player->vel.x * dt;
    f32 new_y = player->pos.y + player->vel.y * dt;

    // If player falls beyond map bottom
    if (new_y >= tm->tiles_high * tm->tile_size) {
        SetGamepadVibration(3, 10.0f, 10.0f, 0.5f);
        state->state = GAME_STATE_GAME_OVER;
    }

    // Horizontal sweep and resolution
    if (player->vel.x != 0.0f) {
        Rectangle horz_box = {
            .x = (player->vel.x > 0) ? player->pos.x + player->size.x : new_x,
            .y = player->pos.y,
            .width = fabsf(player->vel.x * dt),
            .height = player->size.y,
        };

        size_t first, last;
        get_overlapping_tiles(horz_box, &first, &last);

        // TODO: fix the first and last not being correct
        // for (size_t i = first; i <= last; ++i) {
        for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
            Tile* tile = &tm->tiles[i];
            if (!tile->solid) {
                continue;
            }

            if (CheckCollisionRecs(horz_box, tile->dst)) {
                util_debug("collisios");

                if (player->vel.x > 0) {
                    // Resolve to the right
                    new_x = tile->dst.x - player->size.x - 0.001f;
                } else {
                    // Resolve to the left
                    new_x = tile->dst.x + tile->dst.width + 0.001f;
                }
                player->vel.x = 0.0f;
                break;
            }
        }
    }

    // Vertical sweep and resolution
    if (player->vel.y != 0.0f) {
        Rectangle vert_box = {
            .x = new_x,
            .y = (player->vel.y > 0) ? player->pos.y + player->size.y : new_y,
            .width = player->size.x,
            .height = fabsf(player->vel.y * dt),
        };

        size_t first, last;
        get_overlapping_tiles(vert_box, &first, &last);

        player->on_ground = false;

        // TODO: fix the first and last not being correct
        // for (size_t i = first; i <= last; ++i) {
        for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
            Tile* tile = &tm->tiles[i];
            if (!tile->solid) continue;

            if (CheckCollisionRecs(vert_box, tile->dst)) {
                if (player->vel.y > 0) {
                    // Player is falling
                    new_y = tile->dst.y - player->size.y - 0.001f;
                    player->on_ground = true;
                } else {
                    // Player is moving up/jumping
                    new_y = tile->dst.y + tile->dst.height + 0.001f;
                }
                player->vel.y = 0.0f;
                break;
            }
        }
    }

    player->pos.x = new_x;
    player->pos.y = new_y;
    state->camera.target = player->pos;
}

void player_reset(Player* player)
{
    Vector2 player_wpos = screenp_to_worldp(
        (Vector2){
            .x = (f32)GetScreenWidth() * 0.5f,
            .y = (f32)GetScreenHeight() * 0.5f,
        },
        &state->camera,
        (f32)GetScreenWidth(),
        (f32)GetScreenHeight());

    *player = (Player){
        .pos = player_wpos,
        .size =
            {
                .x = 18,
                .y = 18,
            },
        .dir = DIRECTION_RIGHT,
    };
}

void player_render(void)
{
    // Scale added for size in player creation and in update for pos
    DrawRectangleRec(
        (Rectangle){
            .x = player->pos.x,
            .y = player->pos.y,
            .width = player->size.x,
            .height = player->size.y,
        },
        GREEN);

    // --------------------------------------------------------------------------------------------
    // TODO: debug overlap
    //
    f32 dt = GetFrameTime();
    f32 new_x = player->pos.x + player->vel.x * dt;
    Rectangle horz_box = {
        .x = (player->vel.x > 0) ? player->pos.x + player->size.x : new_x,
        .y = player->pos.y,
        .width = fabsf(player->vel.x * dt),
        .height = player->size.y,
    };
    DrawRectangleRec(horz_box, RED);

    size_t first, last;
    get_overlapping_tiles(horz_box, &first, &last);

    for (size_t i = 0; i < n_live_bullets; ++i) {
        Bullet b = bullets[live_bullets[i]];
        u16 y = (u16)(b.pos.y + player->size.y * 0.5f);

        DrawLineEx((Vector2){b.pos.x, y},
                   (Vector2){
                       b.pos.x + 5.0f,
                       y,
                   },
                   5.0f,
                   PALEBLUE_D);
    }
}

// ································································································

static inline void get_overlapping_tiles(Rectangle r, size_t* out_first, size_t* out_last)
{
    // Clamp to map bounds
    int col_start = (int)floorf(r.x / MAP_TILE_SIZE);
    int row_start = (int)floorf(r.y / MAP_TILE_SIZE);
    int col_end = (int)ceilf((r.x + r.width) / MAP_TILE_SIZE);
    int row_end = (int)ceilf((r.y + r.height) / MAP_TILE_SIZE);

    // Clamp to legal indices
    if (col_start < 0) col_start = 0;
    if (row_start < 0) row_start = 0;
    if (col_end > MAP_COL_TILES) col_end = MAP_COL_TILES;
    if (row_end > MAP_ROW_TILES) row_end = MAP_ROW_TILES;

    // Convert the 2‑D region to a 1‑D range (row‑major)
    *out_first = (size_t)(row_start * MAP_COL_TILES + col_start);
    *out_last = (size_t)((row_end * MAP_COL_TILES + col_end) - 1);
}
