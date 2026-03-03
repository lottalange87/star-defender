#ifndef PARTICLE_H
#define PARTICLE_H

#include "math.h"
#include <SDL.h>

#define MAX_PARTICLES 1000

typedef enum {
    PARTICLE_EXPLOSION,
    PARTICLE_THRUSTER,
    PARTICLE_SPARK
} ParticleType;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    float life;
    float max_life;
    float size;
    SDL_Color color;
    ParticleType type;
    int active;
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
    int count;
} ParticleSystem;

void particle_system_init(ParticleSystem* ps);
void particle_spawn(ParticleSystem* ps, Vec2 pos, Vec2 vel, float life, float size, SDL_Color color, ParticleType type);
void particle_spawn_explosion(ParticleSystem* ps, Vec2 pos, SDL_Color color, int count);
void particle_spawn_thruster(ParticleSystem* ps, Vec2 pos, float angle);
void particle_update(ParticleSystem* ps, float dt);
void particle_draw(ParticleSystem* ps, SDL_Renderer* renderer);

#endif
