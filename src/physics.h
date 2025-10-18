#ifndef PHYSICS_H_
#define PHYSICS_H_

#include "utils.h"
#include <stdbool.h>

typedef union {
    f32 vec2[2];
    struct {
        f32 x;
        f32 y;
    };
    struct {
        f32 w;
        f32 h;
    };
} vec2;

typedef union {
    i32 vec2[2];
    struct {
        i32 x;
        i32 y;
    };
    struct {
        i32 w;
        i32 h;
    };
} ivec2;

inline static bool
check_point_rect_collision(const f32 px, const f32 py, const f32 rx, const f32 ry, const f32 rw, const f32 rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

inline static bool check_aabb_collision(const f32 ax,
                                        const f32 ay,
                                        const f32 aw,
                                        const f32 ah,
                                        const f32 bx,
                                        const f32 by,
                                        const f32 bw,
                                        const f32 bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

#endif // !PHYSICS_H_
