#ifndef ENTITY_H_
#define ENTITY_H_

#include "arena.h"
#include "physics.h"
#include "utils.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#define MAX_ENTITIES 32
#define MAX_COMPONENTS 1024

#define SIGNATURE_SET(sig, comp) ((sig) |= (1U << (comp)))
#define SIGNATURE_CLEAR(sig, comp) ((sig) &= ~(1U << (comp)))
#define SIGNATURE_MATCH(entity_sig, system_mask) (((entity_sig) & (system_mask)) == (system_mask))

enum ComponentID {
    COMP_NONE = 0,
    COMP_TRANSFORM,
    COMP_SPRITE,
    COMP_ANIMATION,
    COMP_KEYBOARD_CONTROL,
    COMP_MOUSE_CONTROL,
    COMP_BOX_COLLIDER,
    COMP_RIGID_BODY,
    COMP_CAMERA_FOLLOW,
    COMP_TILEINFO,
    COMP_COUNT = MAX_COMPONENTS,
};

typedef struct {
    vec2 pos;
    vec2 scale;
    f32 rotation;
} TransformComponent;

typedef struct {
    SDL_Texture* texture;
    vec2 size;
    u8 z_index;
    bool is_fixed;
    SDL_FlipMode is_h_flipped;
    SDL_FlipMode is_v_flipped;
    SDL_FRect src;
} SpriteComponent;

typedef u32 Entity;
typedef u32 Signature;

typedef struct EntityManager {
    u32 next_entity_id;
    TransformComponent transform_comps[MAX_ENTITIES];
    SpriteComponent sprite_comps[MAX_ENTITIES];
    Signature signatures[MAX_ENTITIES];
    bool live_entities[MAX_ENTITIES];
} EntityManager;

bool entmgr_init(MemoryArena* mem);
void entity_render(void);
Entity entity_create(void);
void entity_destroy(Entity e);

// Components -------------------------------------------------------------------------------------

void transform_add(Entity e, vec2 pos);
void transform_remove(Entity e);
void sprite_add(Entity e, SDL_Texture* tex, vec2 size, SDL_FRect src, bool is_fixed);
void sprite_remove(Entity e);

#endif // !ENTITY_H_
