#include "tilemap.h"
#include "arena.h"
#include "gfx.h"
#include "input.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <math.h>
#include <stddef.h>

static Tilemap tm;

static void render_map(GameState* state, SDL_Renderer* renderer, const f32 mx, const f32 my);

static void update_edit_mode(GameState* state, const f32 mx, const f32 my);
static void render_edit_mode(GameState* state, SDL_Renderer* renderer, const f32 mx, const f32 my);
static bool update_tileset(const f32 mx, const f32 my);
static void render_tileset(SDL_Renderer* renderer);

bool tilemap_init(MemoryArena* mem, SDL_Texture* tex)
{
    tm.tile_size = MAP_TILE_SIZE;
    tm.width = MAP_COL_TILES;
    tm.height = MAP_ROW_TILES;
    tm.tileset.texture = tex;
    SDL_GetTextureSize(tm.tileset.texture, &tm.tileset.size.w, &tm.tileset.size.h);

    tm.tiles = (Tile*)arena_alloc_aligned(mem, sizeof(Tile), 16);
    if (!tm.tiles) {
        util_error("Failed to allocate for map tiles");
        return false;
    }

    i32 win_w, win_h;
    SDL_GetWindowSize(gfx_get_window(), &win_w, &win_h);

    tm.tileset.pos.x = (f32)(win_w / SCALE) - tm.tileset.size.w - 10;
    tm.tileset.pos.y = (f32)(win_h / SCALE) - tm.tileset.size.h - 10;

    tm.tileset.tile_size = 18;
    tm.tileset.pitch = tm.tileset.size.w / tm.tile_size;

    return true;
}

void tilemap_update(GameState* state)
{
    MouseSnapshot mouse_snapshot = input_get_mouse_snapshot();
    f32 mx = mouse_snapshot.x / SCALE;
    f32 my = mouse_snapshot.y / SCALE;

    if (state->state == GAME_STATE_EDITING) {
        if (update_tileset(mx, my)) return;

        update_edit_mode(state, mx, my);
    }
}

void tilemap_render(GameState* state)
{
    SDL_Renderer* renderer = gfx_get_renderer();
    MouseSnapshot mouse_snapshot = input_get_mouse_snapshot();
    f32 mx = mouse_snapshot.x / SCALE;
    f32 my = mouse_snapshot.y / SCALE;

    // Render positioning grid
    if (state->state == GAME_STATE_EDITING) {
        render_edit_mode(state, renderer, mx, my);
    }

    // Render game map
    render_map(state, renderer, mx, my);

    // Render editing mode UI
    if (state->state == GAME_STATE_EDITING) {
        render_tileset(renderer);
    }
}

// ------------------------------------------------------------------------------------------------

static void render_map(GameState* state, SDL_Renderer* renderer, const f32 mx, const f32 my)
{
    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        SDL_RenderTexture(gfx_get_renderer(), tm.tileset.texture, &tm.tiles[i].src, &tm.tiles[i].dst);
    }
}

static void update_edit_mode(GameState* state, const f32 mx, const f32 my)
{
    u16 gridx = (u16)floorf(mx / tm.tile_size);
    u16 gridy = (u16)floorf(my / tm.tile_size);

    // Set the start location of the brush
    if (input_keypressed(state->input.kb.lshift)) {
        tm.tileset.brush.startpos.x = gridx;
        tm.tileset.brush.startpos.y = gridy;
    }

    // Paint with brush
    if (input_mouse_pressed(INPUT_MOUSE_BTN_LEFT)) {
        tm.tiles[(gridy * tm.width) + gridx] = (Tile){
            .dst =
                {
                    .x = tm.tileset.brush.startpos.x * tm.tileset.tile_size,
                    .y = tm.tileset.brush.startpos.y * tm.tileset.tile_size,
                    .w = tm.tileset.tile_size,
                    .h = tm.tileset.tile_size,
                    // .x = gridx * tm.tile_size,
                    // .y = gridy * tm.tile_size,
                    // .w = tm.tileset.drag_rect.w,
                    // .h = tm.tileset.drag_rect.h,
                },
            .src = tm.tileset.brush.tile.src,
        };
    }
}

static void render_edit_mode(GameState* state, SDL_Renderer* renderer, const f32 mx, const f32 my)
{
    u16 gridx = (u16)floorf(mx / tm.tile_size);
    u16 gridy = (u16)floorf(my / tm.tile_size);

    // Render grid
    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        u16 x = i % tm.width;
        u16 y = i / tm.width;

        SDL_SetRenderDrawColor(renderer, paleblue_des.r, paleblue_des.g, paleblue_des.b, paleblue_des.a);
        SDL_RenderRect(renderer,
                       &(SDL_FRect){
                           .x = x * tm.tile_size,
                           .y = y * tm.tile_size,
                           .w = tm.tile_size,
                           .h = tm.tile_size,
                       });
    }

    if (!tm.tileset.inside) {
        u16 rectw = tm.tile_size;
        u16 recth = tm.tile_size;
        u16 x = gridx;
        u16 y = gridy;

        if (input_keydown(state->input.kb.lshift)) {
            x = tm.tileset.brush.startpos.x;
            y = tm.tileset.brush.startpos.y;

            rectw *= abs(gridx + 1 - tm.tileset.brush.startpos.x);
            recth *= abs(gridy + 1 - tm.tileset.brush.startpos.y);
        }
        // if (input_keyup(state->input.kb.lshift)) {
        //     tm.tileset.drag_rect.w = rectw;
        //     tm.tileset.drag_rect.h = recth;
        // }
        SDL_FRect dst = {
            .x = x * tm.tile_size,
            .y = y * tm.tile_size,
            .w = rectw,
            .h = recth,
        };
        SDL_RenderTexture(renderer, tm.tileset.texture, &tm.tileset.brush.tile.src, &dst);
    }
}

static bool update_tileset(const f32 mx, const f32 my)
{
    tm.tileset.inside = false;

    if (check_point_rect_collision(mx, my, tm.tileset.pos.x, tm.tileset.pos.y, tm.tileset.size.w, tm.tileset.size.h)) {
        tm.tileset.inside = true;

        f32 mouse_relx = mx - tm.tileset.pos.x;
        f32 mouse_rely = my - tm.tileset.pos.y;
        u16 tilex = (int)floorf(mouse_relx / tm.tileset.tile_size);
        u16 tiley = (int)floorf(mouse_rely / tm.tileset.tile_size);

        tm.tileset.hovered_tile.x = tilex;
        tm.tileset.hovered_tile.y = tiley;

        if (input_mouse_pressed(INPUT_MOUSE_BTN_LEFT)) {
            if (!tm.tileset.brush.is_set) {
                tm.tileset.brush.is_set = true;
            }
            tm.tileset.brush.tile.src.x = tm.tileset.hovered_tile.x * tm.tileset.tile_size;
            tm.tileset.brush.tile.src.y = tm.tileset.hovered_tile.y * tm.tileset.tile_size;
            tm.tileset.brush.tile.src.w = tm.tileset.tile_size;
            tm.tileset.brush.tile.src.h = tm.tileset.tile_size;
        }

        return true;
    }

    return false;
}

static void render_tileset(SDL_Renderer* renderer)
{
    SDL_RenderTexture(renderer,
                      tm.tileset.texture,
                      NULL,
                      &(SDL_FRect){
                          .x = tm.tileset.pos.x,
                          .y = tm.tileset.pos.y,
                          .w = tm.tileset.size.w,
                          .h = tm.tileset.size.h,
                      });

    if (tm.tileset.inside) {
        SDL_SetRenderDrawColor(renderer, 0xff, 0x22, 0x22, 0xff);
        SDL_RenderRect(renderer,
                       &(SDL_FRect){
                           .x = tm.tileset.pos.x + (tm.tileset.hovered_tile.x * tm.tileset.tile_size),
                           .y = tm.tileset.pos.y + (tm.tileset.hovered_tile.y * tm.tileset.tile_size),
                           .w = tm.tile_size,
                           .h = tm.tile_size,
                       });
    }

    if (tm.tileset.brush.is_set) {
        u16 brushx = tm.tileset.pos.x + tm.tileset.size.w - tm.tileset.tile_size;
        u16 brushy = tm.tileset.pos.y - tm.tileset.tile_size - 5;

        SDL_FRect dst = {
            .x = brushx,
            .y = brushy,
            .w = tm.tileset.tile_size,
            .h = tm.tileset.tile_size,
        };

        SDL_RenderTexture(renderer, tm.tileset.texture, &tm.tileset.brush.tile.src, &dst);

        SDL_SetRenderDrawColor(renderer, paleblue_d.r, paleblue_d.g, paleblue_d.b, paleblue_d.a);
        SDL_RenderDebugText(renderer, brushx - 45, brushy + 5, "Brush");
    }
}
