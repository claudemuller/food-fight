#include "ui.h"
#include "asset_manager.h"
#include "gfx.h"
#include "level.h"

void render_debug_ui(GameState* state)
{
    Tilemap* tm = &state->active_level->tilemap;
    u32 map_w = tm->tiles_wide * tm->tile_size;
    Font* font = assetmgr_get_font("main");

    DrawTextEx(*font,
               TextFormat("Zoom: x%.2f", state->camera.zoom),
               (Vector2){GetScreenWidth() - 150, 10},
               UI_DEBUG_FONT_SIZE,
               1.0f,
               PALEBLUE_D);
    DrawTextEx(*font,
               TextFormat("STATE: %s", state->state == GAME_STATE_PLAYING ? "playing" : "editing"),
               (Vector2){GetScreenWidth() - 150, 25},
               UI_DEBUG_FONT_SIZE,
               1.0f,
               PALEBLUE_D);

    Vector2 mpos = GetMousePosition();

    Vector2 renpos = mpos;
    if (renpos.x > GetScreenWidth() - 255) {
        renpos.x -= 280;
    }
    renpos.y = mpos.y - 55;
    if (renpos.y <= 5) {
        renpos.y += 85;
    }

    DrawTextEx(*font, TextFormat("screen_pos: %.2f x %.2f", mpos.x, mpos.y), renpos, 18, 1.0f, PALEBLUE_D);

    Vector2 wpos = screenp_to_worldp(mpos, &state->camera, GetScreenWidth(), GetScreenHeight());
    DrawTextEx(*font,
               TextFormat("world_pos: %.2f x %.2f", wpos.x, wpos.y),
               (Vector2){
                   .x = renpos.x,
                   .y = renpos.y + 20,
               },
               UI_TEXT_SIZE,
               1.0f,
               PALEBLUE_D);

    Vector2 grid = worldp_to_gridp((Vector2){mpos.x, mpos.y}, tm->tile_size);

    DrawTextEx(*font,
               TextFormat("grid_pos: %f x %f", grid.x, grid.y),
               (Vector2){
                   .x = renpos.x,
                   .y = renpos.y + 40,
               },
               UI_TEXT_SIZE,
               1.0f,
               PALEBLUE_D);
}
