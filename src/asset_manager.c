#include "asset_manager.h"
#include "arena.h"
#include <SDL3_image/SDL_image.h>

static SDL_Renderer* renderer;
static AssetManager* mgr;
static MemoryArena* game_mem;

bool assetmgr_init(MemoryArena* gmem, SDL_Renderer* r)
{
    mgr = (AssetManager*)arena_alloc_aligned(gmem, sizeof(AssetManager), 16);
    if (!mgr) {
        util_error("Failed to allocate for asset manager");
        return false;
    }

    mgr->count = 0;
    // const char* texture_ids[MAX_TEXTURES];
    // SDL_Texture* textures[MAX_TEXTURES];

    renderer = r;
    game_mem = gmem;

    return true;
}

SDL_Texture* assetmgr_load_texture(const char* fname)
{
    if (!renderer) {
        util_error("Renderer is NULL :(");
        return NULL;
    }

    if (mgr->count >= MAX_TEXTURES) {
        util_error("Asset manager has no more space for textures");
        return NULL;
    }

    for (size_t i = 0; i < mgr->count; ++i) {
        if (strncmp(mgr->texture_ids[i], fname, strlen(fname)) == 0) {
            return mgr->textures[i];
        }
    }

    SDL_Surface* surface = IMG_Load(fname);
    if (!surface) {
        util_error("Failed to load image: %s", SDL_GetError());
        return NULL;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (!tex) {
        util_error("Failed to create texture: %s", SDL_GetError());
        return NULL;
    }
    SDL_DestroySurface(surface);

    // SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

    size_t fnamelen = strlen(fname);
    char* texid = (char*)arena_alloc_aligned(game_mem, fnamelen, 16);
    strncpy(texid, fname, fnamelen);

    mgr->textures[mgr->count] = tex;
    mgr->texture_ids[mgr->count] = texid;
    mgr->count++;

    return tex;
}

SDL_Texture* assetmgr_get_texture(const char* id)
{
    for (size_t i = 0; i < mgr->count; ++i) {
        if (strncmp(mgr->texture_ids[i], id, strlen(id)) == 0) {
            return mgr->textures[i];
        }
    }
    return NULL;
}

void assetmgr_destroy(void)
{
    for (size_t i = 0; i < mgr->count; ++i) {
        SDL_DestroyTexture(mgr->textures[i]);
    }
}
