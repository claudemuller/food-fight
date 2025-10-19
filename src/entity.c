#include "entity.h"
#include "gfx.h"
#include <SDL3/SDL_render.h>
#include <stddef.h>

static EntityManager* entmgr;

bool entmgr_init(MemoryArena* mem)
{
    entmgr = (EntityManager*)arena_alloc_aligned(mem, sizeof(EntityManager), 16);
    if (!entmgr) {
        util_error("Failed to allocate for entity manager");
        return false;
    }

    // 0 is reserved for error handling
    entmgr->next_entity_id = 1;

    return true;
}

Entity entity_create(void)
{
    if (entmgr->next_entity_id >= MAX_ENTITIES) {
        return UINT32_MAX;
    }

    Entity e = entmgr->next_entity_id++;
    entmgr->signatures[e] = 0;
    entmgr->live_entities[e] = true;

    return e;
}

// TODO: replace with systems
void entity_render(void)
{
    for (size_t e = 1; e < entmgr->next_entity_id; ++e) {
        TransformComponent* t = &entmgr->transform_comps[e];
        SpriteComponent* s = &entmgr->sprite_comps[e];

        SDL_FRect dst = (SDL_FRect){
            .x = t->pos.x,  //- (s->is_fixed ? 0 : state->camera.x), // * state->scale,
            .y = t->pos.y,  //- (s->is_fixed ? 0 : state->camera.y), // * state->scale,
            .w = s->size.w, // * state->scale,
            .h = s->size.h, // * state->scale,
        };

        // SDL_RenderTexture(gfx_get_renderer(), s->texture, &s->src, &dst);
    }
}

void entity_destroy(Entity e)
{
    if (e >= MAX_ENTITIES || !entmgr->live_entities[e]) return;

    entmgr->signatures[e] = 0;
    entmgr->live_entities[e] = false;
}

// Components -------------------------------------------------------------------------------------
//
void transform_add(Entity e, vec2 pos)
{
    if (e >= MAX_ENTITIES || !entmgr->live_entities[e]) return;

    entmgr->transform_comps[e] = (TransformComponent){
        .pos =
            {
                .x = pos.x,
                .y = pos.y,
            },
    };

    SIGNATURE_SET(entmgr->signatures[e], COMP_TRANSFORM);
}

void transform_remove(Entity e)
{
    if (e >= MAX_ENTITIES || !entmgr->live_entities[e]) return;
    SIGNATURE_CLEAR(entmgr->signatures[e], COMP_TRANSFORM);
}

void sprite_add(Entity e, SDL_Texture* tex, vec2 size, SDL_FRect src, bool is_fixed)
{
    if (e >= MAX_ENTITIES || !entmgr->live_entities[e]) return;

    entmgr->sprite_comps[e] = (SpriteComponent){
        .texture = tex,
        .size = size,
        .src = src,
        .is_fixed = is_fixed,
    };

    SIGNATURE_SET(entmgr->signatures[e], COMP_SPRITE);
}

void sprite_remove(Entity e)
{
    if (e >= MAX_ENTITIES || entmgr->live_entities[e]) return;
    SIGNATURE_CLEAR(entmgr->signatures[e], COMP_SPRITE);
}
