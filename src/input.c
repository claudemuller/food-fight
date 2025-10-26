#include "input.h"
#include "raylib.h"

static f32 btof(bool b);

void input_process(Input* input)
{
    // Keyboard -----------------------------------------------------------------------------------

    static u32 prev_kb_down = 0;
    u32 new_kb_down = 0;

    new_kb_down |= IsKeyDown(KEY_A) ? KB_A : 0;
    new_kb_down |= IsKeyDown(KEY_S) ? KB_S : 0;
    new_kb_down |= IsKeyDown(KEY_W) ? KB_W : 0;
    new_kb_down |= IsKeyDown(KEY_D) ? KB_D : 0;
    new_kb_down |= IsKeyDown(KEY_SPACE) ? KB_SPACE : 0;
    new_kb_down |= IsKeyDown(KEY_F1) ? KB_F1 : 0;
    new_kb_down |= IsKeyDown(KEY_LEFT_SHIFT) ? KB_LSHFT : 0;
    new_kb_down |= IsKeyDown(KEY_ESCAPE) ? KB_ESCAPE : 0;

    // Derive pressed/released from kb btn transition
    input->kb.pressed = (new_kb_down & ~prev_kb_down);  // 0→1 edges
    input->kb.released = (~new_kb_down & prev_kb_down); // 1→0 edges
    input->kb.down = new_kb_down;                       // current state

    /* Axis – using the helper you already wrote */
    input->kb.axis.x = btof(IsKeyDown(KEY_D)) - btof(IsKeyDown(KEY_A));
    input->kb.axis.y = btof(IsKeyDown(KEY_S)) - btof(IsKeyDown(KEY_W));

    // Store this state for next frame's prev state
    prev_kb_down = new_kb_down;

    // Mouse --------------------------------------------------------------------------------------

    static u32 prev_mouse_down = 0;
    u32 new_mouse_down = 0;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) new_mouse_down |= MB_LEFT;
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) new_mouse_down |= MB_RIGHT;
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) new_mouse_down |= MB_MIDDLE;

    // Derive pressed/released from mouse btn transition
    input->mouse.pressed = (new_mouse_down & ~prev_mouse_down);
    input->mouse.released = (~new_mouse_down & prev_mouse_down);
    input->mouse.down = new_mouse_down;

    input->mouse.pos_px = GetMousePosition();
    input->mouse.wheel_delta = GetMouseWheelMove();

    // Store this state for next frame's prev state
    prev_mouse_down = new_mouse_down;
}

bool input_is_key_down(Keyboard* kb, KeyboardKeys k)
{
    return (kb->down & k) != 0;
}

bool input_is_key_pressed(Keyboard* kb, KeyboardKeys k)
{
    return (kb->pressed & k) != 0;
}

bool input_is_key_released(Keyboard* kb, KeyboardKeys k)
{
    return (kb->released & k) != 0;
}

bool input_is_mouse_down(Mouse* m, MouseButtons b)
{
    return (m->down & b) != 0;
}

bool input_is_mouse_pressed(Mouse* m, MouseButtons b)
{
    return (m->pressed & b) != 0;
}

bool input_is_mouse_released(Mouse* m, MouseButtons b)
{
    return (m->released & b) != 0;
}

void input_reset(Input* input)
{
    input->kb.down = input->kb.pressed = input->kb.released = 0;
    input->mouse.down = input->mouse.pressed = input->mouse.released = 0;
    input->kb.axis = (Vector2){0.0f, 0.0f};
}

// ------------------------------------------------------------------------------------------------

static f32 btof(bool b)
{
    return b ? 1.0 : 0.0;
}
