#ifndef STATE_H_
#define STATE_H_

#include "input.h"
#include "raylib.h"
#include <stdbool.h>

#define WINDOW_WIDTH 1980
#define WINDOW_HEIGHT 1080

#define GRAVITY 600.0f
#define DRAG_COEF 0.0015f
#define TERMINAL_VELOCITY sqrtf(GRAVITY / DRAG_COEF)

#define PLAYER_SPEED 100.0f
#define PLAYER_JUMP_STRENGTH 250.0f

#define MOUSE_ROTATION_DAMPNING 0.025f

#define JUMP_SPEED 10.0f

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_EDITING,
} State;

typedef struct Level Level;

typedef struct GameState {
    Input input;
    State state;
    Camera2D camera;
    Level* active_level;
    bool is_running;
} GameState;

#endif // !STATE_H_
