#ifndef MATH_H
#define MATH_H

#include <math.h>

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    int x, y;
} Vec2i;

typedef struct {
    float x, y, w, h;
} Rect;

static inline Vec2 vec2(float x, float y) {
    return (Vec2){x, y};
}

static inline Vec2 vec2_add(Vec2 a, Vec2 b) {
    return (Vec2){a.x + b.x, a.y + b.y};
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return (Vec2){a.x - b.x, a.y - b.y};
}

static inline Vec2 vec2_mul(Vec2 v, float s) {
    return (Vec2){v.x * s, v.y * s};
}

static inline float vec2_len(Vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline Vec2 vec2_norm(Vec2 v) {
    float len = vec2_len(v);
    if (len < 0.0001f) return (Vec2){0, 0};
    return (Vec2){v.x / len, v.y / len};
}

static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static inline int rect_intersect(Rect a, Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

#endif
