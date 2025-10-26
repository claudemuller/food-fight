#ifndef ARENA_H_
#define ARENA_H_

#include "utils.h"
#include <stddef.h>
#include <stdlib.h>

#define MB (1024 * 1024)

typedef struct {
    unsigned char* base; // The start of the arena
    size_t cap;
    size_t offset;
} MemoryArena;

typedef size_t ArenaMarker;

static inline void arena_init(MemoryArena* arena, size_t cap)
{
    arena->base = (unsigned char*)malloc(cap * sizeof(unsigned char));
    if (!arena->base) {
        util_error("Failed to malloc arena");
    }
    arena->cap = cap;
    arena->offset = 0;
}

static inline size_t align_forward(size_t ptr, size_t align)
{
    size_t misalignment = ptr & (align - 1);
    if (misalignment) ptr += (align - misalignment);
    return ptr;
}

static inline void* arena_alloc_aligned(MemoryArena* arena, size_t size, size_t align)
{
    size_t aligned_offset = align_forward(arena->offset, align);
    if (aligned_offset + size > arena->cap) {
        util_error("No space left in arena: size=%zu, cap=%zu, offset=%zu", size, arena->cap, aligned_offset);
        return NULL;
    }
    void* ptr = arena->base + aligned_offset;
    arena->offset = aligned_offset + size;
    return ptr;
}

static inline ArenaMarker arena_get_marker(MemoryArena* arena)
{
    return arena->offset;
}

static inline void arena_set_marker(MemoryArena* arena, ArenaMarker marker)
{
    arena->offset = marker;
}

static inline void arena_reset(MemoryArena* arena)
{
    arena->offset = 0;
}

static inline void arena_free(MemoryArena* arena)
{
    free(arena->base);
    arena->cap = 0;
    arena->offset = 0;
}

#endif // !ARENA_H_
