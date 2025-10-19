#ifndef STATE_H_
#define STATE_H_

#include "asset_manager.h"
#include "input.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SCALE 2.0f

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_EDITING,
} State;

typedef struct GameState {
    InputState input;
    AssetManager* assetmgr;
    State state;
    bool is_running;
} GameState;

#endif // !STATE_H_
