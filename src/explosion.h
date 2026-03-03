#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "sprite.h"
#include "math.h"
#include <SDL.h>

#define MAX_EXPLOSIONS 50
#define EXPLOSION_FRAMES 8

typedef struct {
    Vec2 pos;
    int frame;
    float timer;
    float frame_time;
    int active;
    float scale;
} Explosion;

typedef struct {
    Explosion explosions[MAX_EXPLOSIONS];
    Sprite* frames[EXPLOSION_FRAMES];
} ExplosionSystem;

void explosion_system_init(ExplosionSystem* es);
void explosion_system_load(ExplosionSystem* es, SDL_Renderer* renderer);
void explosion_system_free(ExplosionSystem* es);
void explosion_spawn(ExplosionSystem* es, Vec2 pos, float scale);
void explosion_update(ExplosionSystem* es, float dt);
void explosion_draw(ExplosionSystem* es, SDL_Renderer* renderer);

#endif
