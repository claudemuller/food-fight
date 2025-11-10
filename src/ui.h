#ifndef UI_H_
#define UI_H_

#include "raylib.h"
#include "utils.h"

#define UI_HEADER_SIZE 40.0f
#define UI_HEADER_SIZE_ALT 20.0f

#define UI_TEXT_SIZE 18.0f

#define UI_EDIT_MODE_SIZE 22.0f

#define UI_MENU_ITEM_SIZE 20.0f

#define UI_DEBUG_FONT_SIZE 16

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
