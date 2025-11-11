#include "input.h"
#include "raylib.h"
#include "utils.h"

static f32 btof(bool b);

void input_process(Input* input)
{
    // Keyboard -----------------------------------------------------------------------------------

    static u32 prev_kb_down = 0;
    u32 new_kb_down = 0;

    new_kb_down |= IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) ? KB_A : 0;
    new_kb_down |= IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) ? KB_S : 0;
    new_kb_down |= IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) ? KB_W : 0;
    new_kb_down |= IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) ? KB_D : 0;
    new_kb_down |= IsKeyDown(KEY_SPACE) ? KB_SPACE : 0;
    new_kb_down |= IsKeyDown(KEY_F1) ? KB_F1 : 0;
    new_kb_down |= IsKeyDown(KEY_F2) ? KB_F2 : 0;
    new_kb_down |= IsKeyDown(KEY_F3) ? KB_F3 : 0;
    new_kb_down |= IsKeyDown(KEY_F4) ? KB_F4 : 0;
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

    input->mouse.pos_px = GetMousePosition();
    input->mouse.wheel_delta = GetMouseWheelMove() * 0.5f;

    static u32 prev_mouse_down = 0;
    u32 new_mouse_down = 0;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) new_mouse_down |= MB_LEFT;
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) new_mouse_down |= MB_RIGHT;
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) new_mouse_down |= MB_MIDDLE;

    // Derive pressed/released from mouse btn transition
    input->mouse.pressed = (new_mouse_down & ~prev_mouse_down);
    if (input->mouse.pressed) {
        input->mouse.down_pos_px = input->mouse.pos_px;
    }
    input->mouse.released = (~new_mouse_down & prev_mouse_down);
    input->mouse.down = new_mouse_down;

    // Store this state for next frame's prev state
    prev_mouse_down = new_mouse_down;

    // for (size_t i = 0; i < 4; ++i) {
    //     util_debug("%d. %s", i, GetGamepadName(i));
    // }
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

// I was lazy 😅
bool input_gamepad_button_pressed(const i32 id, GamepadButton b)
{
    return IsGamepadButtonPressed(id, b);
}

bool input_gamepad_button_released(const i32 id, GamepadButton b)
{
    return IsGamepadButtonReleased(id, b);
}

bool input_gamepad_button_down(const i32 id, GamepadButton b)
{
    return IsGamepadButtonDown(id, b);
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
