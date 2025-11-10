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
create_menu_item(const char* label, const u16 x, const u16 y, const action_fn fn, const TextAlignment align)
{
    u16 posx = x;
    i32 text_w = MeasureText(label, UI_MENU_ITEM_SIZE);

    if (align == ALIGN_CENTRE) {
        posx = GetScreenWidth() * 0.5f - text_w * 0.5f;
    }

    return (MenuItem){
        .label = label,
        .rec =
            {
                .x = posx,
                .y = y,
                .width = text_w,
                .height = UI_MENU_ITEM_SIZE,
            },
        .fn = fn,
        .align = align,
    };
}

#endif // !UI_H_
