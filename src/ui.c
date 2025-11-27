#include "ui.h"
#include "asset_manager.h"
#include "gfx.h"
#include "level.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui/floating_window.h"
#include "raygui/gui_window_file_dialog.h"
#include "raygui/raygui.h"

static GameState* state;

void ui_init(GameState* game_state)
{
    state = game_state;
    GuiLoadStyle("assets/styles/light.rgs");
}

bool ui_is_hovering(const Vector2 p, Rectangle r)
{
    bool hovered = CheckCollisionPointRec(p, r);
    if (hovered) {
        state->ui_hovered = true;
    }
    return hovered;
}

void render_debug_ui(GameState* state)
{
    Tilemap* tm = &state->active_level->tilemap;
    // u32 map_w = tm->tiles_wide * tm->tile_size;
    Font* font = assetmgr_get_font("main");

    DrawTextEx(*font,
               TextFormat("Zoom: x%.2f", state->camera.zoom),
               (Vector2){10.0f, 10.0f},
               UI_DEBUG_FONT_SIZE,
               1.0f,
               PALEBLUE_D);
    DrawTextEx(*font,
               TextFormat("STATE: %s", state->state == GAME_STATE_PLAYING ? "playing" : "editing"),
               (Vector2){10.0f, 25.0f},
               UI_DEBUG_FONT_SIZE,
               1.0f,
               PALEBLUE_D);
    DrawTextEx(*font,
               TextFormat("ui_active: %s [%d]", state->ui_hovered ? "true" : "false", state->ui_hovered),
               (Vector2){10.0f, 40.0f},
               UI_DEBUG_FONT_SIZE,
               1.0f,
               PALEBLUE_D);

    Vector2 mpos = GetMousePosition();

    Vector2 renpos = mpos;
    if (renpos.x > (f32)GetScreenWidth() - 255) {
        renpos.x -= 280;
    }
    renpos.y = mpos.y - 55;
    if (renpos.y <= 5) {
        renpos.y += 85;
    }

    DrawTextEx(*font, TextFormat("screen_pos: %.2f x %.2f", mpos.x, mpos.y), renpos, 18, 1.0f, PALEBLUE_D);

    Vector2 wpos = screenp_to_worldp(mpos, &state->camera, (f32)GetScreenWidth(), (f32)GetScreenHeight());
    DrawTextEx(*font,
               TextFormat("world_pos: %.2f x %.2f", wpos.x, wpos.y),
               (Vector2){
                   .x = renpos.x,
                   .y = renpos.y + 20,
               },
               UI_TEXT_SIZE,
               1.0f,
               PALEBLUE_D);

    Vector2 grid = worldp_to_gridp((Vector2){mpos.x, mpos.y}, (u8)tm->tile_size);

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

void message_box_content(Vector2 position, Vector2 scroll)
{
    GuiButton((Rectangle){position.x + 20 + scroll.x, position.y + 50 + scroll.y, 100, 25}, "Button 1");
    GuiButton((Rectangle){position.x + 20 + scroll.x, position.y + 100 + scroll.y, 100, 25}, "Button 2");
}

void message_box(const char* title, const char* _)
{
    Vector2 window_position = {10, 10};
    Vector2 window_size = {200, 400};
    Vector2 scroll = {200, 400};
    bool minimized = false;
    bool moving = false;
    bool resizing = false;

    GuiWindowFloating(&window_position,
                      &window_size,
                      &minimized,
                      &moving,
                      &resizing,
                      &message_box_content,
                      (Vector2){140, 320},
                      &scroll,
                      title);
}

bool ui_draw_image_button(const Vector2 pos, const f32 size, const char* tex_id, const char* hint)
{
    Texture2D* tex = assetmgr_get_texture(tex_id);
    if (!tex) {
        tex = assetmgr_load_texture(tex_id);
        if (!tex) {
            util_error("Failed to load button texture");
            return false;
        }
    }

    Rectangle dst = {
        .x = pos.x,
        .y = pos.y,
        .width = size,
        .height = size,
    };

    if (ui_is_hovering(GetMousePosition(), dst)) {
        f32 txt_len = (f32)MeasureText(hint, UI_HINT_SIZE);
        f32 x = pos.x + (size * 0.5f) - (txt_len * 0.5f);
        f32 y = pos.y + size + 10.0f;
        DrawText(hint, (i32)x, (i32)y, UI_HINT_SIZE, PALEBLUE_D);

        DrawRectangleLinesEx(dst, 2.0f, RED);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            return true;
        }
    }

    DrawRectangleRec(dst, (Color){0, 150, 0, 100});

    Rectangle src = {
        .width = (f32)tex->width,
        .height = (f32)tex->height,
    };

    DrawTexturePro(*tex, src, dst, (Vector2){0}, 0.0f, WHITE);

    return false;
}

bool ui_draw_button(const Vector2 pos, const Vector2 size, const Color bgcolor, const Color hover_color)
{
    Rectangle btn_rec = {
        .x = pos.x,
        .y = pos.y,
        .width = size.x,
        .height = size.y,
    };
    Color c = bgcolor;

    if (ui_is_hovering(GetMousePosition(), btn_rec)) {
        c = hover_color;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            return true;
        }
    }

    DrawRectangleRec(btn_rec, c);

    return false;
}

// ------------------------------------------------------------------------------------------------
