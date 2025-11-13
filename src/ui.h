#ifndef UI_H_
#define UI_H_

#include "raylib.h"
#include "state.h"
#include "utils.h"

#define UI_PADDING 10.0f
#define UI_HEADER_SIZE 40.0f
#define UI_HEADER_SIZE_ALT 20.0f

#define UI_TEXT_SIZE 18.0f
#define UI_HINT_SIZE 14.0f

#define UI_EDIT_MODE_SIZE 22.0f

#define UI_MENU_ITEM_SIZE 20.0f

#define UI_DEBUG_FONT_SIZE 16.0f

typedef enum {
    ALIGN_NONE,
    ALIGN_LEFT,
    ALIGN_CENTRE,
    ALIGN_RIGHT,
} TextAlignment;

typedef void (*action_fn)(void);

typedef struct {
    Rectangle rec;
    const char* label;
    action_fn fn;
    TextAlignment align;
    bool hover;
} MenuItem;

bool ui_get_hovered(void);
void ui_set_hovered(const bool hovered);
void render_debug_ui(GameState* state);
bool ui_draw_image_button(const Vector2 pos, const f32 size, const char* tex_id, const char* hint);
bool ui_draw_button(const Vector2 pos, const Vector2 size, const Color bgcolor, const Color hover_color);

static inline MenuItem
create_menu_item(const char* label, const u16 x, const u16 y, Font* font, const action_fn fn, const TextAlignment align)
{
    u16 posx = x;
    Vector2 text_size = MeasureTextEx(*font, label, UI_MENU_ITEM_SIZE, 1.0f);

    if (align == ALIGN_CENTRE) {
        posx = GetScreenWidth() * 0.5f - text_size.x * 0.5f;
    }

    return (MenuItem){
        .label = label,
        .rec =
            {
                .x = posx,
                .y = y,
                .width = text_size.x,
                .height = text_size.y,
            },
        .fn = fn,
        .align = align,
    };
}

#endif // !UI_H_
