#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include "arena.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#define MAX_TEXTURES 15

typedef struct AssetManager {
    size_t count;
    const char* texture_ids[MAX_TEXTURES];
    SDL_Texture* textures[MAX_TEXTURES];
} AssetManager;

bool assetmgr_init(MemoryArena* game_mem, SDL_Renderer* r);
SDL_Texture* assetmgr_load_texture(const char* fname);
SDL_Texture* assetmgr_get_texture(const char* id);
void assetmgr_destroy(void);

#endif // !ASSET_MANAGER_H_
