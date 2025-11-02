#ifndef UTILS_H_
#define UTILS_H_

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define util_error(fmt, ...) util_err("🛑 ERROR [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define util_info(fmt, ...) util_inf("ℹ️ INFO: " fmt "\n", ##__VA_ARGS__)
#define util_warn(fmt, ...) util_inf("⚠️ WARN [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define util_debug(fmt, ...) util_inf("🔍 DEBUG [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define util_fatal(fmt, ...) util_fat("💀 FATAL [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#pragma clang diagnostic pop

static inline void util_inf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);
}

static inline void util_err(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);
}

static inline void util_fat(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);

    exit(EXIT_FAILURE);
}

static inline u32 min(u32 a, u32 b)
{
    return a < b ? a : b;
}

static inline u32 max(u32 a, u32 b)
{
    return a > b ? a : b;
}

static inline u32 clamp(u32 v, u32 lo, u32 hi)
{
    return min(max(v, lo), hi);
}

static inline f32 clampf(f32 v, f32 lo, f32 hi)
{
    return fminf(fmaxf(v, lo), hi);
}

#endif // UTILS_H_
