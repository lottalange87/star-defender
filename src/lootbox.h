#ifndef LOOTBOX_H
#define LOOTBOX_H

#include "math.h"
#include "particle.h"
#include "entity.h"
#include <SDL.h>

#define MAX_LOOTBOXES 10

typedef enum {
    LOOT_WEAPON_UPGRADE,
    LOOT_SHIELD,
    LOOT_SMART_BOMB,
    LOOT_SCORE_MULTIPLIER,
    LOOT_EXTRA_LIFE,
    LOOT_RAPID_FIRE,
    NUM_LOOT_TYPES
} LootType;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    float rotation;
    float bob_offset;
    LootType type;
    int active;
    float lifetime;
    int opened;
} LootBox;

typedef struct {
    LootBox boxes[MAX_LOOTBOXES];
    float score_multiplier;
    float score_mult_timer;
    float rapid_fire_timer;
    int shield_active;
} LootSystem;

void loot_system_init(LootSystem* ls);
void loot_spawn(LootSystem* ls, Vec2 pos);
void loot_update(LootSystem* ls, float dt, EntityManager* em, ParticleSystem* ps);
void loot_draw(LootSystem* ls, SDL_Renderer* renderer);
void loot_collect(LootBox* box, Entity* player, LootSystem* ls, EntityManager* em, ParticleSystem* ps);

// Apply loot effects
void loot_apply_weapon_upgrade(Entity* player);
void loot_apply_shield(Entity* player, LootSystem* ls);
void loot_apply_smart_bomb(EntityManager* em, ParticleSystem* ps);
void loot_apply_score_multiplier(LootSystem* ls);
void loot_apply_extra_life(Entity* player);
void loot_apply_rapid_fire(Entity* player, LootSystem* ls);

// Visuals
void loot_draw_box(SDL_Renderer* renderer, LootBox* box);
void loot_draw_icon(SDL_Renderer* renderer, LootType type, float x, float y, float scale);
const char* loot_get_name(LootType type);

#endif
