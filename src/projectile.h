#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "math.h"
#include <SDL.h>

#define MAX_PROJECTILE_VISUALS 100

typedef struct {
    Vec2 pos;
    Vec2 vel;
    float rotation;
    float rot_speed;
    int type; // 0 = player, 1 = enemy
    int active;
    float life;
    float pulse;
} ProjectileVisual;

typedef struct {
    ProjectileVisual visuals[MAX_PROJECTILE_VISUALS];
} ProjectileSystem;

void projectile_system_init(ProjectileSystem* ps);
void projectile_spawn_visual(ProjectileSystem* ps, Vec2 pos, Vec2 vel, int type);
void projectile_update_visuals(ProjectileSystem* ps, float dt);
void projectile_draw_visuals(ProjectileSystem* ps, SDL_Renderer* renderer);

// Draw a single fancy projectile
void draw_fancy_projectile(SDL_Renderer* renderer, float x, float y, float rotation, int type, float pulse);

#endif
