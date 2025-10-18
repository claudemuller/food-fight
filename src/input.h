#ifndef INPUT_H_
#define INPUT_H_

#include "utils.h"
#include <stdbool.h>

typedef enum {
    INPUT_KEY_UP,
    INPUT_KEY_PRESSED,
    INPUT_KEY_DOWN,
} KeyState;

typedef struct {
    KeyState up;
    KeyState down;
    KeyState left;
    KeyState right;
    KeyState escape;
    KeyState f1;
} Keyboard;

typedef enum {
    INPUT_MOUSE_BTN_LEFT,
    INPUT_MOUSE_BTN_MIDDLE,
    INPUT_MOUSE_BTN_RIGHT,
} MouseBtn;

typedef enum {
    BTN_UP,      // Button not pressed this frame
    BTN_PRESSED, // Transitioned from up to down this frame
    BTN_HELD,    // Was already down in the previous frame and stays down
    BTN_RELEASED // Transitioned from down to up this frame
} BtnTransition;

typedef struct {
    BtnTransition transition;
} MouseInfo;

typedef struct {
    f32 x;
    f32 y;
    MouseInfo left;
    MouseInfo middle;
    MouseInfo right;
} MouseSnapshot;

typedef struct {
    MouseInfo left;
    MouseInfo middle;
    MouseInfo right;
    u32 prev_btn_mask;
} Mouse;

typedef struct InputState {
    Keyboard kb;
} InputState;

typedef struct GameState GameState;

bool input_init(void);
void input_process(GameState* state);
bool input_keydown(KeyState ks);
bool input_keypressed(KeyState ks);
bool input_keyup(KeyState ks);
MouseSnapshot input_get_mouse_snapshot(void);
bool input_mouse_pressed(MouseBtn btn);

#endif // !INPUT_H_
