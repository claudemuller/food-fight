#ifndef RAYGUI_FLOATING_WINDOW_H_
#define RAYGUI_FLOATING_WINDOW_H_

#include <raylib.h>

void GuiWindowFloating(Vector2* position,
                       Vector2* size,
                       bool* minimized,
                       bool* moving,
                       bool* resizing,
                       void (*draw_content)(Vector2, Vector2),
                       Vector2 content_size,
                       Vector2* scroll,
                       const char* title);

#endif // !RAYGUI_FLOATING_WINDOW_H_
