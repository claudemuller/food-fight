#ifndef INPUT_H_
#define INPUT_H_

#include "utils.h"
#include <raylib.h>
#include <stdbool.h>

typedef enum {
    KB_NONE = 0U,
    KB_A = 1U << 0,       // 0x0000_0000_0000_0001
    KB_S = 1U << 1,       // 0x0000_0000_0000_0010
    KB_W = 1U << 2,       // 0x0000_0000_0000_0100
    KB_D = 1U << 3,       // 0x0000_0000_0000_1000
    KB_Q = 1U << 4,       // 0x0000_0000_0001_0000
    KB_E = 1U << 5,       // 0x0000_0000_0010_0000
    KB_SPACE = 1U << 6,   // 0x0000_0000_0100_0000
    KB_F1 = 1U << 7,      // 0x0000_0000_1000_0000
    KB_F2 = 1U << 8,      // 0x0000_0001_0000_0000
    KB_F3 = 1U << 9,      // 0x0000_0010_0000_0000
    KB_LSHFT = 1U << 10,  // 0x0000_0100_0000_0000
    KB_ESCAPE = 1U << 11, // 0x0000_1000_0000_0000
} KeyboardKeys;

typedef struct {
    u64 down;
    u64 pressed;
    u64 released;
    Vector2 axis;
} Keyboard;

typedef enum {
    MB_LEFT = 1U << 0,   // 0x0001
    MB_MIDDLE = 1U << 1, // 0x0010
    MB_RIGHT = 1U << 2,  // 0x0100
} MouseButtons;

typedef struct {
    u8 down;
    u8 pressed;
    u8 released;
    f32 wheel_delta;
    Vector2 down_pos_px;
    Vector2 pos_px;
} Mouse;

typedef struct {
    Keyboard kb;
    Mouse mouse;
} Input;

void input_process(Input* input);
bool input_is_key_down(Keyboard* kb, KeyboardKeys k);
bool input_is_key_pressed(Keyboard* kb, KeyboardKeys k);
bool input_is_key_released(Keyboard* kb, KeyboardKeys k);
bool input_is_mouse_down(Mouse* m, MouseButtons b);
bool input_is_mouse_pressed(Mouse* m, MouseButtons b);
bool input_is_mouse_released(Mouse* m, MouseButtons b);
bool input_gamepad_button_pressed(const i32 id, GamepadButton b);
bool input_gamepad_button_released(const i32 id, GamepadButton b);
bool input_gamepad_button_down(const i32 id, GamepadButton b);
void input_reset(Input* input);

#endif // !INPUT_H_
