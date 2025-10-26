#include "asset_manager.h"
#include "arena.h"
#include <raylib.h>
#include <stdbool.h>
#include <string.h>

static AssetManager* mgr;
static MemoryArena* game_mem;

bool assetmgr_init(MemoryArena* gmem)
{
    mgr = (AssetManager*)arena_alloc_aligned(gmem, sizeof(AssetManager), 16);
    if (!mgr) {
        util_error("Failed to allocate for asset manager");
        return false;
    }

    mgr->n_textures = 0;
    game_mem = gmem;

    return true;
}

Texture2D* assetmgr_load_texture(const char* fname)
{
    if (mgr->n_textures >= MAX_TEXTURES) {
        util_error("Asset manager has no more space for textures");
        return NULL;
    }

    // Check if texture already exisits
    for (size_t i = 0; i < mgr->n_textures; ++i) {
        if (strncmp(mgr->texture_ids[i], fname, strlen(fname)) == 0) {
            return &mgr->textures[i];
        }
    }

    mgr->textures[mgr->n_textures] = LoadTexture(fname);
    if (!IsTextureValid(mgr->textures[mgr->n_textures])) {
        util_error("Failed to load texture");
        return NULL;
    }

    size_t fnamelen = strlen(fname);
    char* texid = (char*)arena_alloc_aligned(game_mem, fnamelen, 16);
    strncpy(texid, fname, fnamelen);

    mgr->texture_ids[mgr->n_textures] = texid;
    mgr->n_textures++;

    return &mgr->textures[mgr->n_textures - 1];
}

Texture2D* assetmgr_get_texture(const char* id)
{
    for (size_t i = 0; i < mgr->n_textures; ++i) {
        if (strncmp(mgr->texture_ids[i], id, strlen(id)) == 0) {
            return &mgr->textures[i];
        }
    }
    return NULL;
}

void assetmgr_destroy(void)
{
    for (size_t i = 0; i < mgr->n_textures; ++i) {
        UnloadTexture(mgr->textures[i]);
    }
}
