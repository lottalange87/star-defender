#include "entity.h"
#include <stdlib.h>
#include <math.h>

void entity_manager_init(EntityManager* em) {
    em->entity_count = 0;
    em->projectile_count = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        em->entities[i].active = 0;
    }
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        em->projectiles[i].active = 0;
    }
}

Entity* entity_spawn(EntityManager* em, EntityType type, float x, float y) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!em->entities[i].active) {
            Entity* e = &em->entities[i];
            e->type = type;
            e->pos = vec2(x, y);
            e->active = 1;
            e->frame = 0;
            e->anim_timer = 0;
            e->shoot_timer = 0;
            e->rotation = 0;
            e->sprite = NULL;
            
            switch (type) {
                case ENTITY_PLAYER:
                    e->speed = 300.0f;
                    e->hp = 3;
                    e->max_hp = 3;
                    e->width = 32;
                    e->height = 32;
                    e->weapon_level = 1;
                    break;
                case ENTITY_ENEMY_NORMAL:
                    e->speed = 100.0f;
                    e->hp = 1;
                    e->max_hp = 1;
                    e->width = 28;
                    e->height = 28;
                    e->score_value = 10;
                    e->vel = vec2(0, 80.0f);
                    break;
                case ENTITY_ENEMY_FAST:
                    e->speed = 180.0f;
                    e->hp = 1;
                    e->max_hp = 1;
                    e->width = 24;
                    e->height = 24;
                    e->score_value = 20;
                    e->vel = vec2(50.0f, 120.0f);
                    break;
                case ENTITY_ENEMY_TANK:
                    e->speed = 60.0f;
                    e->hp = 3;
                    e->max_hp = 3;
                    e->width = 40;
                    e->height = 40;
                    e->score_value = 50;
                    e->vel = vec2(0, 50.0f);
                    break;
                case ENTITY_POWERUP:
                    e->speed = 50.0f;
                    e->hp = 1;
                    e->width = 20;
                    e->height = 20;
                    e->vel = vec2(0, 30.0f);
                    break;
                default:
                    break;
            }
            
            em->entity_count++;
            return e;
        }
    }
    return NULL;
}

void entity_destroy(EntityManager* em, int index) {
    if (index >= 0 && index < MAX_ENTITIES && em->entities[index].active) {
        em->entities[index].active = 0;
        em->entity_count--;
    }
}

void projectile_spawn(EntityManager* em, EntityType type, float x, float y, float vx, float vy) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!em->projectiles[i].active) {
            Entity* p = &em->projectiles[i];
            p->type = type;
            p->pos = vec2(x, y);
            p->vel = vec2(vx, vy);
            p->active = 1;
            p->width = 10;
            p->height = 30;
            p->sprite = NULL;
            p->rotation = atan2f(vy, vx) * 180.0f / 3.14159f + 90.0f;
            em->projectile_count++;
            return;
        }
    }
}

void entity_update(EntityManager* em, float dt, ParticleSystem* ps, int screen_w, int screen_h) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &em->entities[i];
        if (!e->active) continue;
        
        // Movement
        e->pos = vec2_add(e->pos, vec2_mul(e->vel, dt));
        
        // Animation
        e->anim_timer += dt;
        if (e->anim_timer > 0.1f) {
            e->anim_timer = 0;
            e->frame++;
        }
        
        // Enemy AI
        if (e->type >= ENTITY_ENEMY_NORMAL && e->type <= ENTITY_ENEMY_TANK) {
            // Zigzag for fast enemies
            if (e->type == ENTITY_ENEMY_FAST) {
                e->pos.x += sinf(e->pos.y * 0.01f) * 100.0f * dt;
            }
            
            // Shooting
            e->shoot_timer += dt;
            if (e->shoot_timer > 2.0f + (rand() % 100) / 100.0f) {
                e->shoot_timer = 0;
                projectile_spawn(em, ENTITY_PROJECTILE_ENEMY, e->pos.x, e->pos.y + e->height/2, 0, 200.0f);
            }
            
            // Remove if off screen
            if (e->pos.y > screen_h + 50) {
                e->active = 0;
                em->entity_count--;
            }
        }
        
        // Powerup movement
        if (e->type == ENTITY_POWERUP) {
            if (e->pos.y > screen_h + 50) {
                e->active = 0;
                em->entity_count--;
            }
        }
    }
}

void entity_draw(EntityManager* em, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &em->entities[i];
        if (!e->active) continue;
        
        if (e->sprite) {
            sprite_draw(renderer, e->sprite, e->frame % e->sprite->frame_count,
                       e->pos.x - e->width/2, e->pos.y - e->height/2, 1.0f, e->rotation, SDL_FLIP_NONE);
        } else {
            // Fallback: colored rectangles
            SDL_Rect rect = {(int)(e->pos.x - e->width/2), (int)(e->pos.y - e->height/2),
                           (int)e->width, (int)e->height};
            
            switch (e->type) {
                case ENTITY_PLAYER:
                    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
                    break;
                case ENTITY_ENEMY_NORMAL:
                    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
                    break;
                case ENTITY_ENEMY_FAST:
                    SDL_SetRenderDrawColor(renderer, 255, 80, 255, 255);
                    break;
                case ENTITY_ENEMY_TANK:
                    SDL_SetRenderDrawColor(renderer, 255, 200, 80, 255);
                    break;
                case ENTITY_POWERUP:
                    SDL_SetRenderDrawColor(renderer, 80, 255, 80, 255);
                    break;
                default:
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void projectile_update(EntityManager* em, float dt, int screen_w, int screen_h) {
    em->projectile_count = 0;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Entity* p = &em->projectiles[i];
        if (!p->active) continue;
        
        p->pos = vec2_add(p->pos, vec2_mul(p->vel, dt));
        
        // Remove if off screen
        if (p->pos.x < -50 || p->pos.x > screen_w + 50 ||
            p->pos.y < -50 || p->pos.y > screen_h + 50) {
            p->active = 0;
        } else {
            em->projectile_count++;
        }
    }
}

void projectile_draw(EntityManager* em, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Entity* p = &em->projectiles[i];
        if (!p->active) continue;
        
        SDL_Rect rect = {(int)(p->pos.x - p->width/2), (int)(p->pos.y - p->height/2),
                        (int)p->width, (int)p->height};
        
        if (p->type == ENTITY_PROJECTILE_PLAYER) {
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        }
        SDL_RenderFillRect(renderer, &rect);
    }
}

int entity_check_collision(Entity* a, Entity* b) {
    Rect ra = {a->pos.x - a->width/2, a->pos.y - a->height/2, a->width, a->height};
    Rect rb = {b->pos.x - b->width/2, b->pos.y - b->height/2, b->width, b->height};
    return rect_intersect(ra, rb);
}

void entity_take_damage(Entity* e, int damage, ParticleSystem* ps) {
    e->hp -= damage;
    
    // Spawn hit particles
    SDL_Color hit_color = {255, 200, 100, 200};
    particle_spawn_explosion(ps, e->pos, hit_color, 5);
    
    if (e->hp <= 0) {
        // Death explosion
        SDL_Color explosion_color;
        switch (e->type) {
            case ENTITY_ENEMY_NORMAL:
                explosion_color = (SDL_Color){255, 100, 100, 255};
                break;
            case ENTITY_ENEMY_FAST:
                explosion_color = (SDL_Color){255, 100, 255, 255};
                break;
            case ENTITY_ENEMY_TANK:
                explosion_color = (SDL_Color){255, 200, 100, 255};
                break;
            default:
                explosion_color = (SDL_Color){200, 200, 200, 255};
        }
        particle_spawn_explosion(ps, e->pos, explosion_color, 20);
        e->active = 0;
    }
}
