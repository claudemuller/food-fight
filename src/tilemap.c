#include "tilemap.h"
#include "entity.h"
#include "gfx.h"
#include "input.h"
#include "utils.h"
#include <SDL3/SDL_rect.h>
#include <math.h>

static Tilemap tm;

static bool update_tileset(const f32 mx, const f32 my);

bool tilemap_init(SDL_Texture* tex)
{
    tm.tile_size = 18;
    tm.width = 10;
    tm.height = 10;
    tm.tileset.texture = tex;
    SDL_GetTextureSize(tm.tileset.texture, &tm.tileset.size.w, &tm.tileset.size.h);

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

    if (input_mouse_pressed(INPUT_MOUSE_BTN_LEFT)) {
        Entity tile = entity_create();
        SDL_FRect src = {
            .x = tm.tileset.selected.tile.x * tm.tileset.tile_size,
            .y = tm.tileset.selected.tile.y * tm.tileset.tile_size,
            .w = tm.tileset.tile_size,
            .h = tm.tileset.tile_size,
        };
        transform_add(tile, (vec2){.x = mx - (tm.tile_size * 0.5f), .y = my - (tm.tile_size * 0.5f)});
        sprite_add(tile, tm.tileset.texture, (vec2){.w = tm.tile_size, .h = tm.tile_size}, src, false);
    }
}

void tilemap_render_tileset(void)
{
    SDL_Renderer* renderer = gfx_get_renderer();

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

    if (tm.tileset.selected.selected) {
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

// ------------------------------------------------------------------------------------------------

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
            if (!tm.tileset.selected.selected) {
                tm.tileset.selected.selected = true;
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
