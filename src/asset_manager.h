#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include "arena.h"
#include <raylib.h>
#include <stdbool.h>

#define MAX_TEXTURES 15

typedef struct AssetManager {
    size_t n_textures;
    const char* texture_ids[MAX_TEXTURES];
    Texture2D textures[MAX_TEXTURES];
} AssetManager;

bool assetmgr_init(MemoryArena* game_mem);
Texture2D* assetmgr_load_texture(const char* fname);
Texture2D* assetmgr_get_texture(const char* id);
void assetmgr_destroy(void);

#endif // !ASSET_MANAGER_H_
