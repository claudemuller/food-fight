#include "tilemap.h"
#include "arena.h"
#include "entity.h"
#include "gfx.h"
#include "input.h"
#include "utils.h"
#include <SDL3/SDL_rect.h>
#include <math.h>
#include <stddef.h>

static Tilemap tm;

static void update_tilemap(const f32 mx, const f32 my);
static void render_tilemap(SDL_Renderer* renderer);
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

    if (update_tileset(mx, my)) return;
    update_tilemap(mx, my);
}

void tilemap_render_tileset(void)
{
    SDL_Renderer* renderer = gfx_get_renderer();
    render_tilemap(renderer);
    render_tileset(renderer);
}

// ------------------------------------------------------------------------------------------------

static void update_tilemap(const f32 mx, const f32 my)
{
    if (input_mouse_pressed(INPUT_MOUSE_BTN_LEFT)) {
        u16 x = (u16)floorf(mx / tm.width);
        u16 y = (u16)floorf(my / tm.width);

        Entity tile = entity_create();
        SDL_FRect src = {
            .x = tm.tileset.selected.tile.x * tm.tileset.tile_size,
            .y = tm.tileset.selected.tile.y * tm.tileset.tile_size,
            .w = tm.tileset.tile_size,
            .h = tm.tileset.tile_size,
        };
        transform_add(tile, (vec2){.x = x * tm.tile_size, .y = y * tm.tile_size});
        sprite_add(tile, tm.tileset.texture, (vec2){.w = tm.tile_size, .h = tm.tile_size}, src, false);
    }
}

static void render_tilemap(SDL_Renderer* renderer)
{
    MouseSnapshot mouse_snapshot = input_get_mouse_snapshot();
    u16 mx = (u16)floorf(mouse_snapshot.x / tm.width);
    u16 my = (u16)floorf(mouse_snapshot.y / tm.width);

    for (size_t i = 0; i < MAX_NUM_TILES; ++i) {
        u16 x = i / tm.width;
        u16 y = i % tm.width;

        if (x == mx && y == my) {
            SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
        } else {
            SDL_SetRenderDrawColor(renderer, paleblue_des.r, paleblue_des.g, paleblue_des.b, paleblue_des.a);
        }

        SDL_RenderRect(renderer,
                       &(SDL_FRect){
                           .x = x * tm.tile_size,
                           .y = y * tm.tile_size,
                           .w = tm.tile_size,
                           .h = tm.tile_size,
                       });
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
            if (!tm.tileset.selected.is_selected) {
                tm.tileset.selected.is_selected = true;
            }
            tm.tileset.selected.tile.x = tm.tileset.hovered_tile.x;
            tm.tileset.selected.tile.y = tm.tileset.hovered_tile.y;

            // TODO: check if on selected tile, toggle if so
            // tm.selected.selected = !tm.selected.selected;
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
        SDL_SetRenderDrawColor(gfx_get_renderer(), 0xff, 0x22, 0x22, 0xff);

        SDL_RenderRect(renderer,
                       &(SDL_FRect){
                           .x = tm.tileset.pos.x + (tm.tileset.hovered_tile.x * tm.tileset.tile_size),
                           .y = tm.tileset.pos.y + (tm.tileset.hovered_tile.y * tm.tileset.tile_size),
                           .w = tm.tile_size,
                           .h = tm.tile_size,
                       });
    }

    if (tm.tileset.selected.is_selected) {
        SDL_FRect src = {
            .x = tm.tileset.selected.tile.x * tm.tileset.tile_size,
            .y = tm.tileset.selected.tile.y * tm.tileset.tile_size,
            .w = tm.tileset.tile_size,
            .h = tm.tileset.tile_size,
        };

        u16 brushx = tm.tileset.pos.x + tm.tileset.size.w - tm.tileset.tile_size;
        u16 brushy = tm.tileset.pos.y - tm.tileset.tile_size - 5;

        SDL_FRect dst = {
            .x = brushx,
            .y = brushy,
            .w = tm.tileset.tile_size,
            .h = tm.tileset.tile_size,
        };

        SDL_RenderTexture(renderer, tm.tileset.texture, &src, &dst);

        SDL_SetRenderDrawColor(renderer, paleblue_d.r, paleblue_d.g, paleblue_d.b, paleblue_d.a);
        SDL_RenderDebugText(renderer, brushx - 45, brushy + 5, "Brush");
    }
}
