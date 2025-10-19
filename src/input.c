#include "input.h"
#include "state.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

static Uint32 prev_mouse_mask;
static MouseSnapshot curr_mouse_snapshot;

static void set_key_state(bool cur_down, KeyState* key_state);
static BtnTransition bool_to_transition(bool cur_down, bool prev_down);

bool input_init(void)
{
    // if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
    //     ERROR_RETURN(-1, "Controller not be intialised\n");
    // }
    //
    // uint8_t num_joysticks = SDL_NumJoysticks();
    // LOG_INFO("input:gamepad", "Number of joysticks: %d", num_joysticks);
    //
    // if (num_joysticks > 0) {
    //     // NOTE: we only handle one controller
    //     if (SDL_IsGameController(0)) {
    //         controller = SDL_GameControllerOpen(0);
    //         if (controller) {
    //             LOG_INFO("input:gamepad", "Opened game controller: %s", SDL_GameControllerName(controller));
    //         } else {
    //             LOG_INFO("input:gamepad", "Could not open game controller 0: %s", SDL_GetError());
    //         }
    //     } else {
    //         LOG_INFO("input:gamepad", "Joystick is not a game controller.");
    //     }
    // }

    return true;
}

void input_process(GameState* state)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_EVENT_QUIT: {
            state->is_running = false;
        } break;

            // case SDL_EVENT_MOUSE_MOTION: {
            //     state->input.mouse.x = ev.motion.x;
            //     state->input.mouse.y = ev.motion.y;
            // } break;
            //
            // case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            //     set_key_state(true, &state->input.mouse.btn_left);
            // } break;
            //
            // case SDL_EVENT_MOUSE_BUTTON_UP: {
            //     set_key_state(false, &state->input.mouse.btn_left);
            // } break;
        }
    }

    const bool* kb_state = SDL_GetKeyboardState(NULL);

    set_key_state(kb_state[SDL_SCANCODE_UP], &state->input.kb.up);
    set_key_state(kb_state[SDL_SCANCODE_DOWN], &state->input.kb.down);
    set_key_state(kb_state[SDL_SCANCODE_LEFT], &state->input.kb.left);
    set_key_state(kb_state[SDL_SCANCODE_RIGHT], &state->input.kb.right);
    set_key_state(kb_state[SDL_SCANCODE_ESCAPE], &state->input.kb.escape);
    set_key_state(kb_state[SDL_SCANCODE_F1], &state->input.kb.f1);
    set_key_state(kb_state[SDL_SCANCODE_LSHIFT], &state->input.kb.lshift);

    f32 mx, my;
    u32 curr_mouse_mask = SDL_GetMouseState(&mx, &my);

    bool left_now = (curr_mouse_mask & SDL_BUTTON_LMASK) != 0;
    bool middle_now = (curr_mouse_mask & SDL_BUTTON_MMASK) != 0;
    bool right_now = (curr_mouse_mask & SDL_BUTTON_RMASK) != 0;

    bool left_prev = (prev_mouse_mask & SDL_BUTTON_LMASK) != 0;
    bool middle_prev = (prev_mouse_mask & SDL_BUTTON_MMASK) != 0;
    bool right_prev = (prev_mouse_mask & SDL_BUTTON_RMASK) != 0;

    curr_mouse_snapshot.left.transition = bool_to_transition(left_now, left_prev);
    curr_mouse_snapshot.middle.transition = bool_to_transition(middle_now, middle_prev);
    curr_mouse_snapshot.right.transition = bool_to_transition(right_now, right_prev);

    curr_mouse_snapshot.x = mx;
    curr_mouse_snapshot.y = my;

    prev_mouse_mask = curr_mouse_mask;
}

MouseSnapshot input_get_mouse_snapshot(void)
{
    return curr_mouse_snapshot;
}

bool input_mouse_pressed(MouseBtn btn)
{
    switch (btn) {
    case INPUT_MOUSE_BTN_LEFT: {
        return curr_mouse_snapshot.left.transition == BTN_PRESSED;
    } break;

    case INPUT_MOUSE_BTN_MIDDLE: {
        return curr_mouse_snapshot.left.transition == BTN_PRESSED;
    } break;

    case INPUT_MOUSE_BTN_RIGHT: {
        return curr_mouse_snapshot.left.transition == BTN_PRESSED;
    } break;

    default: {
        return false;
    } break;
    }
}

bool input_keydown(KeyState ks)
{
    return ks == INPUT_KEY_DOWN;
}

bool input_keypressed(KeyState ks)
{
    return ks == INPUT_KEY_PRESSED;
}

bool input_keyup(KeyState ks)
{
    return ks == INPUT_KEY_UP;
}

// ---------------------------------------------------------------------------------------------------------------------

static BtnTransition bool_to_transition(bool cur_down, bool prev_down)
{
    if (cur_down) {
        return prev_down ? BTN_HELD : BTN_PRESSED;
    } else {
        return prev_down ? BTN_RELEASED : BTN_UP;
    }
}

static void set_key_state(bool cur_down, KeyState* key_state)
{
    if (cur_down) {
        // If already pressed or held
        if (*key_state > 0) {
            *key_state = INPUT_KEY_DOWN;
        } else {
            *key_state = INPUT_KEY_PRESSED;
        }
    } else {
        *key_state = INPUT_KEY_UP;
    }
}
