#include "explosion.h"
#include "sprite_gen.h"
#include <stdlib.h>

void explosion_system_init(ExplosionSystem* es) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        es->explosions[i].active = 0;
    }
    for (int i = 0; i < EXPLOSION_FRAMES; i++) {
        es->frames[i] = NULL;
    }
}

void explosion_system_load(ExplosionSystem* es, SDL_Renderer* renderer) {
    for (int i = 0; i < EXPLOSION_FRAMES; i++) {
        SDL_Surface* surf = gen_explosion_frame(i);
        es->frames[i] = sprite_from_surface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

void explosion_system_free(ExplosionSystem* es) {
    for (int i = 0; i < EXPLOSION_FRAMES; i++) {
        sprite_free(es->frames[i]);
        es->frames[i] = NULL;
    }
}

void explosion_spawn(ExplosionSystem* es, Vec2 pos, float scale) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!es->explosions[i].active) {
            es->explosions[i].pos = pos;
            es->explosions[i].frame = 0;
            es->explosions[i].timer = 0;
            es->explosions[i].frame_time = 0.05f; // 50ms per frame
            es->explosions[i].active = 1;
            es->explosions[i].scale = scale;
            return;
        }
    }
}

void explosion_update(ExplosionSystem* es, float dt) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (es->explosions[i].active) {
            es->explosions[i].timer += dt;
            
            if (es->explosions[i].timer >= es->explosions[i].frame_time) {
                es->explosions[i].timer = 0;
                es->explosions[i].frame++;
                
                if (es->explosions[i].frame >= EXPLOSION_FRAMES) {
                    es->explosions[i].active = 0;
                }
            }
        }
    }
}

void explosion_draw(ExplosionSystem* es, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (es->explosions[i].active) {
            int frame = es->explosions[i].frame;
            if (es->frames[frame]) {
                sprite_draw(renderer, es->frames[frame], 0,
                           es->explosions[i].pos.x, es->explosions[i].pos.y,
                           es->explosions[i].scale, 0, SDL_FLIP_NONE);
            }
        }
    }
}
