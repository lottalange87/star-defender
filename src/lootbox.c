#include "lootbox.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

void loot_system_init(LootSystem* ls) {
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        ls->boxes[i].active = 0;
    }
    ls->score_multiplier = 1.0f;
    ls->score_mult_timer = 0;
    ls->rapid_fire_timer = 0;
    ls->shield_active = 0;
}

void loot_spawn(LootSystem* ls, Vec2 pos) {
    // 5% chance to spawn from enemy death
    if ((rand() % 100) >= 5) return;
    
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        if (!ls->boxes[i].active) {
            ls->boxes[i].pos = pos;
            ls->boxes[i].vel = vec2(0, 30.0f); // Fall slowly
            ls->boxes[i].rotation = 0;
            ls->boxes[i].bob_offset = (float)(rand() % 100) / 100.0f * 6.28f;
            ls->boxes[i].type = (LootType)(rand() % NUM_LOOT_TYPES);
            ls->boxes[i].active = 1;
            ls->boxes[i].lifetime = 0;
            ls->boxes[i].opened = 0;
            return;
        }
    }
}

void loot_update(LootSystem* ls, float dt, EntityManager* em, ParticleSystem* ps) {
    // Update timers
    if (ls->score_mult_timer > 0) {
        ls->score_mult_timer -= dt;
        if (ls->score_mult_timer <= 0) {
            ls->score_multiplier = 1.0f;
        }
    }
    
    if (ls->rapid_fire_timer > 0) {
        ls->rapid_fire_timer -= dt;
    }
    
    // Update loot boxes
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        LootBox* box = &ls->boxes[i];
        if (!box->active) continue;
        
        box->lifetime += dt;
        
        // Bobbing motion
        box->bob_offset += dt * 3.0f;
        
        // Movement
        box->pos.y += box->vel.y * dt;
        box->pos.x += sinf(box->bob_offset) * 0.5f;
        
        // Rotation
        box->rotation += dt * 30.0f;
        
        // Remove if off screen
        if (box->pos.y > 650) {
            box->active = 0;
        }
    }
}

void loot_collect(LootBox* box, Entity* player, LootSystem* ls, EntityManager* em, ParticleSystem* ps) {
    if (box->opened) return;
    box->opened = 1;
    
    // Spawn collection particles
    SDL_Color collect_color = {255, 255, 100, 255};
    particle_spawn_explosion(ps, box->pos, collect_color, 15);
    
    // Apply effect
    switch (box->type) {
        case LOOT_WEAPON_UPGRADE:
            loot_apply_weapon_upgrade(player);
            break;
        case LOOT_SHIELD:
            loot_apply_shield(player, ls);
            break;
        case LOOT_SMART_BOMB:
            loot_apply_smart_bomb(em, ps);
            break;
        case LOOT_SCORE_MULTIPLIER:
            loot_apply_score_multiplier(ls);
            break;
        case LOOT_EXTRA_LIFE:
            loot_apply_extra_life(player);
            break;
        case LOOT_RAPID_FIRE:
            loot_apply_rapid_fire(player, ls);
            break;
        default:
            break;
    }
    
    box->active = 0;
}

void loot_apply_weapon_upgrade(Entity* player) {
    if (player && player->weapon_level < 3) {
        player->weapon_level++;
    }
}

void loot_apply_shield(Entity* player, LootSystem* ls) {
    (void)player;
    ls->shield_active = 1;
}

void loot_apply_smart_bomb(EntityManager* em, ParticleSystem* ps) {
    // Destroy all enemies on screen
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &em->entities[i];
        if (e->active && e->type >= ENTITY_ENEMY_NORMAL && 
            e->type <= ENTITY_ENEMY_TANK) {
            // Create explosion
            SDL_Color explosion_color = {255, 200, 100, 255};
            particle_spawn_explosion(ps, e->pos, explosion_color, 20);
            e->active = 0;
            em->entity_count--;
        }
    }
}

void loot_apply_score_multiplier(LootSystem* ls) {
    ls->score_multiplier = 2.0f;
    ls->score_mult_timer = 10.0f; // 10 seconds
}

void loot_apply_extra_life(Entity* player) {
    if (player) {
        player->hp++;
    }
}

void loot_apply_rapid_fire(Entity* player, LootSystem* ls) {
    (void)player;
    ls->rapid_fire_timer = 5.0f; // 5 seconds
}

const char* loot_get_name(LootType type) {
    switch (type) {
        case LOOT_WEAPON_UPGRADE: return "WEAPON UP!";
        case LOOT_SHIELD: return "SHIELD";
        case LOOT_SMART_BOMB: return "SMART BOMB!";
        case LOOT_SCORE_MULTIPLIER: return "2X SCORE";
        case LOOT_EXTRA_LIFE: return "EXTRA LIFE";
        case LOOT_RAPID_FIRE: return "RAPID FIRE";
        default: return "???";
    }
}

void loot_draw_box(SDL_Renderer* renderer, LootBox* box) {
    if (!box->active || box->opened) return;
    
    float bob = sinf(box->bob_offset) * 3.0f;
    float x = box->pos.x;
    float y = box->pos.y + bob;
    float size = 24.0f;
    
    // Glow effect
    for (int i = 3; i >= 0; i--) {
        float glow_size = size + i * 6.0f;
        Uint8 alpha = 60 - i * 15;
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, alpha); // Gold glow
        SDL_Rect glow = {
            (int)(x - glow_size/2),
            (int)(y - glow_size/2),
            (int)glow_size,
            (int)glow_size
        };
        SDL_RenderFillRect(renderer, &glow);
    }
    
    // Box
    SDL_SetRenderDrawColor(renderer, 218, 165, 32, 255); // Goldenrod
    SDL_Rect box_rect = {
        (int)(x - size/2),
        (int)(y - size/2),
        (int)size,
        (int)size
    };
    SDL_RenderFillRect(renderer, &box_rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &box_rect);
    
    // Question mark (simple representation)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect q_top = {(int)(x - 3), (int)(y - 8), 6, 4};
    SDL_Rect q_mid = {(int)(x - 3), (int)(y - 2), 6, 2};
    SDL_Rect q_dot = {(int)(x - 1), (int)(y + 4), 2, 2};
    SDL_RenderFillRect(renderer, &q_top);
    SDL_RenderFillRect(renderer, &q_mid);
    SDL_RenderFillRect(renderer, &q_dot);
}

void loot_draw_icon(SDL_Renderer* renderer, LootType type, float x, float y, float scale) {
    // Draw icon based on type
    switch (type) {
        case LOOT_WEAPON_UPGRADE:
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
            break;
        case LOOT_SHIELD:
            SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
            break;
        case LOOT_SMART_BOMB:
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
            break;
        case LOOT_SCORE_MULTIPLIER:
            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
            break;
        case LOOT_EXTRA_LIFE:
            SDL_SetRenderDrawColor(renderer, 255, 100, 255, 255);
            break;
        case LOOT_RAPID_FIRE:
            SDL_SetRenderDrawColor(renderer, 255, 150, 50, 255);
            break;
        default:
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }
    
    float s = 10.0f * scale;
    SDL_Rect icon = {(int)(x - s/2), (int)(y - s/2), (int)s, (int)s};
    SDL_RenderFillRect(renderer, &icon);
}

void loot_draw(LootSystem* ls, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_LOOTBOXES; i++) {
        if (ls->boxes[i].active) {
            loot_draw_box(renderer, &ls->boxes[i]);
        }
    }
}
