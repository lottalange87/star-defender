#ifndef ENTITY_H
#define ENTITY_H

#include "math.h"
#include "sprite.h"
#include "particle.h"
#include "projectile.h"
#include <SDL.h>

#define MAX_ENTITIES 100
#define MAX_PROJECTILES 50

// Callback for spawning visual projectiles
typedef void (*SpawnVisualProjectileFunc)(Vec2 pos, Vec2 vel, int type);
extern SpawnVisualProjectileFunc g_spawn_visual_projectile;

void entity_set_visual_projectile_callback(SpawnVisualProjectileFunc func);

typedef enum {
    ENTITY_PLAYER,
    ENTITY_ENEMY_NORMAL,
    ENTITY_ENEMY_FAST,
    ENTITY_ENEMY_TANK,
    ENTITY_PROJECTILE_PLAYER,
    ENTITY_PROJECTILE_ENEMY,
    ENTITY_POWERUP
} EntityType;

typedef struct {
    EntityType type;
    Vec2 pos;
    Vec2 vel;
    float speed;
    int hp;
    int max_hp;
    float width, height;
    Sprite* sprite;
    int frame;
    float anim_timer;
    float shoot_timer;
    int active;
    float rotation;
    int weapon_level;
    int score_value;
    
    // Visual effects
    float damage_flash;     // Flash timer when hit
    float muzzle_flash;     // Muzzle flash timer when shooting
} Entity;

typedef struct {
    Entity entities[MAX_ENTITIES];
    Entity projectiles[MAX_PROJECTILES];
    int entity_count;
    int projectile_count;
} EntityManager;

void entity_manager_init(EntityManager* em);
Entity* entity_spawn(EntityManager* em, EntityType type, float x, float y);
void entity_destroy(EntityManager* em, int index);
void projectile_spawn(EntityManager* em, EntityType type, float x, float y, float vx, float vy);
void entity_update(EntityManager* em, float dt, ParticleSystem* ps, int screen_w, int screen_h);
void entity_draw(EntityManager* em, SDL_Renderer* renderer);
void projectile_update(EntityManager* em, float dt, int screen_w, int screen_h);
void projectile_draw(EntityManager* em, SDL_Renderer* renderer);

// Collision
int entity_check_collision(Entity* a, Entity* b);
void entity_take_damage(Entity* e, int damage, ParticleSystem* ps);

#endif
