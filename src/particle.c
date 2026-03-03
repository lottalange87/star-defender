#include "particle.h"
#include <math.h>
#include <stdlib.h>

void particle_system_init(ParticleSystem* ps) {
    ps->count = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        ps->particles[i].active = 0;
    }
}

void particle_spawn(ParticleSystem* ps, Vec2 pos, Vec2 vel, float life, float size, SDL_Color color, ParticleType type) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->particles[i].active) {
            ps->particles[i].pos = pos;
            ps->particles[i].vel = vel;
            ps->particles[i].life = life;
            ps->particles[i].max_life = life;
            ps->particles[i].size = size;
            ps->particles[i].color = color;
            ps->particles[i].type = type;
            ps->particles[i].active = 1;
            ps->count++;
            return;
        }
    }
}

void particle_spawn_explosion(ParticleSystem* ps, Vec2 pos, SDL_Color color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 50.0f + (rand() % 150);
        Vec2 vel = vec2(cosf(angle) * speed, sinf(angle) * speed);
        float life = 0.3f + (rand() % 100) / 200.0f;
        float size = 2.0f + (rand() % 6);
        particle_spawn(ps, pos, vel, life, size, color, PARTICLE_EXPLOSION);
    }
}

void particle_spawn_thruster(ParticleSystem* ps, Vec2 pos, float angle) {
    float spread = 0.3f;
    float a = angle + ((rand() % 100) / 100.0f - 0.5f) * spread;
    float speed = 100.0f + (rand() % 100);
    Vec2 vel = vec2(cosf(a) * speed, sinf(a) * speed);
    
    SDL_Color colors[] = {
        {255, 200, 50, 200},
        {255, 150, 30, 180},
        {255, 100, 20, 160},
        {200, 200, 255, 140}
    };
    SDL_Color color = colors[rand() % 4];
    
    particle_spawn(ps, pos, vel, 0.2f, 3.0f + (rand() % 4), color, PARTICLE_THRUSTER);
}

void particle_update(ParticleSystem* ps, float dt) {
    ps->count = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->particles[i].active) {
            Particle* p = &ps->particles[i];
            
            p->pos = vec2_add(p->pos, vec2_mul(p->vel, dt));
            p->life -= dt;
            
            // Drag
            p->vel = vec2_mul(p->vel, 0.98f);
            
            if (p->life <= 0) {
                p->active = 0;
            } else {
                ps->count++;
            }
        }
    }
}

void particle_draw(ParticleSystem* ps, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->particles[i].active) {
            Particle* p = &ps->particles[i];
            
            float alpha = p->life / p->max_life;
            SDL_Color c = p->color;
            c.a = (Uint8)(c.a * alpha);
            
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            
            int size = (int)(p->size * alpha);
            SDL_Rect rect = {
                (int)(p->pos.x - size/2),
                (int)(p->pos.y - size/2),
                size,
                size
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
