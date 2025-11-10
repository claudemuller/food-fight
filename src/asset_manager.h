#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include "arena.h"
#include <raylib.h>
#include <stdbool.h>

#define MAX_TEXTURES 15
#define MAX_FONTS 5

typedef struct AssetManager {
    size_t n_textures;
    size_t n_fonts;
    const char* font_ids[MAX_FONTS];
    Font fonts[MAX_FONTS];
    const char* texture_ids[MAX_TEXTURES];
    Texture2D textures[MAX_TEXTURES];
} AssetManager;

bool assetmgr_init(MemoryArena* game_mem);
Texture2D* assetmgr_load_texture(const char* fname);
Texture2D* assetmgr_get_texture(const char* id);
Font* assetmgr_load_font(const char* fname, const char* id);
Font* assetmgr_get_font(const char* id);
void assetmgr_destroy(void);

#endif // !ASSET_MANAGER_H_
