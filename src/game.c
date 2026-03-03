#include "game.h"
#include "sprite_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Wrapper to spawn visual projectiles from entity.c
static Game* g_current_game = NULL;

static void spawn_visual_wrapper(Vec2 pos, Vec2 vel, int type) {
    if (g_current_game) {
        projectile_spawn_visual(&g_current_game->projectiles_visual, pos, vel, type);
        // Play enemy laser sound
        if (type == 1) {
            audio_play_sound(&g_current_game->audio, SOUND_LASER_ENEMY);
        }
    }
}

int game_init(Game* game, int screen_w, int screen_h, SDL_Renderer* renderer) {
    game->screen_width = screen_w;
    game->screen_height = screen_h;
    game->state = STATE_MENU;
    
    entity_manager_init(&game->entities);
    particle_system_init(&game->particles);
    explosion_system_init(&game->explosions);
    projectile_system_init(&game->projectiles_visual);
    explosion_system_load(&game->explosions, renderer);
    
    // Initialize systems
    audio_init(&game->audio);
    ui_init(&game->ui);
    loot_system_init(&game->loot);
    background_init(&game->background, screen_w, screen_h);
    menu_init(&game->menu, screen_w, screen_h);
    transition_init(&game->transition);
    game->use_menu = 1;
    
    // Set up visual projectile callback
    g_current_game = game;
    entity_set_visual_projectile_callback(spawn_visual_wrapper);
    
    // Generate sprites
    SDL_Surface* surf;
    
    surf = gen_player_ship();
    game->sprite_player = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_enemy_normal();
    game->sprite_enemy_normal = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_enemy_fast();
    game->sprite_enemy_fast = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_enemy_tank();
    game->sprite_enemy_tank = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_projectile_player();
    game->sprite_projectile_player = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_projectile_enemy();
    game->sprite_projectile_enemy = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    surf = gen_powerup();
    game->sprite_powerup = sprite_from_surface(renderer, surf);
    SDL_FreeSurface(surf);
    
    // Assign sprites to entities
    for (int i = 0; i < MAX_ENTITIES; i++) {
        game->entities.entities[i].sprite = NULL;
    }
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        game->entities.projectiles[i].sprite = NULL;
    }
    
    // Init stars
    for (int i = 0; i < 100; i++) {
        game->stars[i].x = rand() % screen_w;
        game->stars[i].y = rand() % screen_h;
        game->stars[i].speed = 20.0f + (rand() % 100);
        game->stars[i].size = 1 + (rand() % 2);
    }
    
    game_reset(game);
    return 1;
}

void game_shutdown(Game* game) {
    ui_shutdown(&game->ui);
    audio_shutdown(&game->audio);
    // loot_system has no shutdown needed
    
    sprite_free(game->sprite_player);
    sprite_free(game->sprite_enemy_normal);
    sprite_free(game->sprite_enemy_fast);
    sprite_free(game->sprite_enemy_tank);
    sprite_free(game->sprite_projectile_player);
    sprite_free(game->sprite_projectile_enemy);
    sprite_free(game->sprite_powerup);
    explosion_system_free(&game->explosions);
}

void game_reset(Game* game) {
    game->score = 0;
    game->lives = 3;
    game->wave = 1;
    game->wave_timer = 0;
    game->spawn_timer = 0;
    game->shoot_cooldown = 0;
    
    entity_manager_init(&game->entities);
    particle_system_init(&game->particles);
    
    // Spawn player
    game->player = entity_spawn(&game->entities, ENTITY_PLAYER, 
                                 game->screen_width / 2.0f, 
                                 game->screen_height - 80.0f);
    if (game->player) {
        game->player->sprite = game->sprite_player;
    }
}

void game_handle_input(Game* game, SDL_Event* event) {
    int down = (event->type == SDL_KEYDOWN);
    
    // Menu input handling
    if (game->state == STATE_MENU && game->use_menu) {
        if (down) {
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_W:
                    menu_move_up(&game->menu);
                    break;
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_S:
                    menu_move_down(&game->menu);
                    break;
                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_SPACE:
                    {
                        MenuItem selected = menu_select(&game->menu);
                        switch (selected) {
                            case MENU_ITEM_SINGLE_PLAYER:
                                game->use_menu = 0;
                                game->state = STATE_PLAYING;
                                game_reset(game);
                                transition_start(&game->transition, TRANSITION_STAR_WARP, 0.5f);
                                break;
                            case MENU_ITEM_TWO_PLAYER:
                                // TODO: Implement two player mode
                                game->use_menu = 0;
                                game->state = STATE_PLAYING;
                                game_reset(game);
                                break;
                            case MENU_ITEM_QUIT:
                                // Handle quit in main loop
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case SDL_SCANCODE_ESCAPE:
                    // Handle quit
                    break;
                default:
                    break;
            }
        }
        return; // Don't process game input in menu
    }
    
    // Game input handling
    switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
            game->key_left = down;
            break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
            game->key_right = down;
            break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
            game->key_up = down;
            break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
            game->key_down = down;
            break;
        case SDL_SCANCODE_SPACE:
            game->key_shoot = down;
            break;
        case SDL_SCANCODE_RETURN:
            if (down && game->state == STATE_GAMEOVER) {
                game_reset(game);
                game->state = STATE_PLAYING;
                transition_start(&game->transition, TRANSITION_FADE_IN, 0.3f);
            }
            break;
        case SDL_SCANCODE_ESCAPE:
            if (down) {
                if (game->state == STATE_PLAYING) game->state = STATE_PAUSE;
                else if (game->state == STATE_PAUSE) game->state = STATE_PLAYING;
            }
            break;
        default:
            break;
    }
}

void game_update(Game* game, float dt) {
    if (game->state != STATE_PLAYING) return;
    
    if (game->player && game->player->active) {
        Vec2 move = vec2(0, 0);
        if (game->key_left) move.x -= 1;
        if (game->key_right) move.x += 1;
        if (game->key_up) move.y -= 1;
        if (game->key_down) move.y += 1;
        
        if (move.x != 0 || move.y != 0) {
            move = vec2_norm(move);
            game->player->pos = vec2_add(game->player->pos, 
                                           vec2_mul(move, game->player->speed * dt));
            
            game->player->pos.x = fmaxf(20, fminf(game->screen_width - 20, game->player->pos.x));
            game->player->pos.y = fmaxf(20, fminf(game->screen_height - 20, game->player->pos.y));
            
            particle_spawn_thruster(
                &game->particles, 
                vec2(game->player->pos.x, game->player->pos.y + 16),
                3.14159f / 2.0f);
        }
        
        game->shoot_cooldown -= dt;
        float fire_rate = (game->loot.rapid_fire_timer > 0) ? 0.05f : 0.15f;
        if (game->key_shoot && game->shoot_cooldown <= 0) {
            game->shoot_cooldown = fire_rate;
            
            switch (game->player->weapon_level) {
                case 1:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x, game->player->pos.y - 16),
                                    vec2(0, -500.0f), 0);
                    game->player->muzzle_flash = 1.0f;
                    audio_play_sound(&game->audio, SOUND_LASER_PLAYER);
                    break;
                case 2:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x - 8, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x + 8, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x - 8, game->player->pos.y - 16),
                                    vec2(0, -500.0f), 0);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x + 8, game->player->pos.y - 16),
                                    vec2(0, -500.0f), 0);
                    game->player->muzzle_flash = 1.0f;
                    audio_play_sound(&game->audio, SOUND_LASER_PLAYER);
                    break;
                case 3:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x - 12, game->player->pos.y - 12, -50.0f, -480.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x + 12, game->player->pos.y - 12, 50.0f, -480.0f);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x, game->player->pos.y - 16),
                                    vec2(0, -500.0f), 0);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x - 12, game->player->pos.y - 12),
                                    vec2(-50.0f, -480.0f), 0);
                    projectile_spawn_visual(&game->projectiles_visual,
                                    vec2(game->player->pos.x + 12, game->player->pos.y - 12),
                                    vec2(50.0f, -480.0f), 0);
                    game->player->muzzle_flash = 1.0f;
                    audio_play_sound(&game->audio, SOUND_LASER_PLAYER);
                    break;
            }
        }
    } else {
        game->state = STATE_GAMEOVER;
    }
    
    game->spawn_timer -= dt;
    if (game->spawn_timer <= 0) {
        game->spawn_timer = 1.5f - (game->wave * 0.1f);
        if (game->spawn_timer < 0.5f) game->spawn_timer = 0.5f;
        
        int x = 40 + (rand() % (game->screen_width - 80));
        
        int roll = rand() % 100;
        EntityType type;
        Sprite* sprite;
        
        if (roll < 60) {
            type = ENTITY_ENEMY_NORMAL;
            sprite = game->sprite_enemy_normal;
        } else if (roll < 85) {
            type = ENTITY_ENEMY_FAST;
            sprite = game->sprite_enemy_fast;
        } else {
            type = ENTITY_ENEMY_TANK;
            sprite = game->sprite_enemy_tank;
        }
        
        Entity* e = entity_spawn(&game->entities, type, x, -30);
        if (e) {
            e->sprite = sprite;
        }
    }
    
    game->wave_timer += dt;
    if (game->wave_timer > 30.0f) {
        game->wave_timer = 0;
        game->wave++;
    }
    
    entity_update(&game->entities, dt, &game->particles, game->screen_width, game->screen_height);
    projectile_update(&game->entities, dt, game->screen_width, game->screen_height);
    particle_update(&game->particles, dt);
    explosion_update(&game->explosions, dt);
    projectile_update_visuals(&game->projectiles_visual, dt);
    loot_update(&game->loot, dt, &game->entities, &game->particles);
    background_update(&game->background, dt, game->screen_width, game->screen_height);
    transition_update(&game->transition, dt);
    
    // Update menu if active
    if (game->state == STATE_MENU && game->use_menu) {
        menu_update(&game->menu, dt);
    }
    
    game_check_collisions(game);
}

void game_draw(Game* game, SDL_Renderer* renderer) {
    // Draw based on state
    if (game->state == STATE_MENU && game->use_menu) {
        // Draw menu with animated background
        menu_draw(&game->menu, renderer, game->screen_width, game->screen_height);
    } else {
        // Draw game with new background
        background_draw(&game->background, renderer, game->screen_width, game->screen_height);
        
        // Draw game entities
        entity_draw(&game->entities, renderer);
        loot_draw(&game->loot, renderer);
        projectile_draw_visuals(&game->projectiles_visual, renderer);
        particle_draw(&game->particles, renderer);
        explosion_draw(&game->explosions, renderer);
        
        // HUD
        if (game->state == STATE_PLAYING) {
            ui_draw_hud(&game->ui, renderer, game->score, game->lives, 
                       game->wave, game->player ? game->player->weapon_level : 1,
                       game->screen_width);
        }
        
        // Overlay screens
        if (game->state == STATE_GAMEOVER) {
            ui_draw_gameover(&game->ui, renderer, game->score, 
                            game->screen_width, game->screen_height);
        } else if (game->state == STATE_PAUSE) {
            ui_draw_pause(&game->ui, renderer, game->screen_width, game->screen_height);
        }
    }
    
    // Draw transition effect
    transition_draw(&game->transition, renderer, game->screen_width, game->screen_height);
}

void game_check_collisions(Game* game) {
    // Player projectiles vs enemies
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        Entity* proj = &game->entities.projectiles[p];
        if (!proj->active || proj->type != ENTITY_PROJECTILE_PLAYER) continue;
        
        for (int e = 0; e < MAX_ENTITIES; e++) {
            Entity* enemy = &game->entities.entities[e];
            if (!enemy->active || enemy->type < ENTITY_ENEMY_NORMAL || 
                enemy->type > ENTITY_ENEMY_TANK) continue;
            
            if (entity_check_collision(proj, enemy)) {
                proj->active = 0;
                
                // Spawn explosion
                float scale = 1.0f;
                if (enemy->type == ENTITY_ENEMY_TANK) scale = 1.5f;
                explosion_spawn(&game->explosions, enemy->pos, scale);
                
                entity_take_damage(enemy, 1, &game->particles);
                
                // Play explosion sound
                if (enemy->type == ENTITY_ENEMY_TANK) {
                    audio_play_sound(&game->audio, SOUND_EXPLOSION_LARGE);
                } else {
                    audio_play_sound(&game->audio, SOUND_EXPLOSION_SMALL);
                }
                
                if (!enemy->active) {
                    game->score += enemy->score_value * (int)game->loot.score_multiplier;
                    
                    // Spawn loot box
                    loot_spawn(&game->loot, enemy->pos);
                    
                    // Weapon upgrade every 500 points
                    if (game->score / 500 > (game->score - enemy->score_value) / 500) {
                        if (game->player->weapon_level < 3) {
                            game->player->weapon_level++;
                        }
                    }
                }
                break;
            }
        }
    }
    
    // Enemy projectiles vs player
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        Entity* proj = &game->entities.projectiles[p];
        if (!proj->active || proj->type != ENTITY_PROJECTILE_ENEMY) continue;
        
        if (game->player && game->player->active && entity_check_collision(proj, game->player)) {
            proj->active = 0;
            explosion_spawn(&game->explosions, game->player->pos, 0.8f);
            audio_play_sound(&game->audio, SOUND_EXPLOSION_SMALL);
            entity_take_damage(game->player, 1, &game->particles);
            game->lives--;
            
            if (game->player->hp <= 0 && game->lives > 0) {
                game->player->hp = 3;
                game->player->pos.x = game->screen_width / 2.0f;
                game->player->pos.y = game->screen_height - 80.0f;
                game->player->active = 1;
                game->player->weapon_level = 1;
            }
            break;
        }
    }
    
    // Loot boxes vs player
    if (game->player && game->player->active) {
        for (int i = 0; i < MAX_LOOTBOXES; i++) {
            LootBox* box = &game->loot.boxes[i];
            if (!box->active || box->opened) continue;
            
            // Simple distance check
            float dx = box->pos.x - game->player->pos.x;
            float dy = box->pos.y - game->player->pos.y;
            float dist = sqrtf(dx*dx + dy*dy);
            
            if (dist < 30.0f) { // Collection radius
                loot_collect(box, game->player, &game->loot, &game->entities, &game->particles);
            }
        }
    }
    
    // Enemies vs player (collision)
    for (int e = 0; e < MAX_ENTITIES; e++) {
        Entity* enemy = &game->entities.entities[e];
        if (!enemy->active || enemy->type < ENTITY_ENEMY_NORMAL || 
            enemy->type > ENTITY_ENEMY_TANK) continue;
        
        if (game->player && game->player->active && entity_check_collision(enemy, game->player)) {
            explosion_spawn(&game->explosions, enemy->pos, 1.2f);
            entity_take_damage(enemy, 10, &game->particles);
            entity_take_damage(game->player, 1, &game->particles);
            game->lives--;
            
            if (game->player->hp <= 0 && game->lives > 0) {
                game->player->hp = 3;
                game->player->pos.x = game->screen_width / 2.0f;
                game->player->pos.y = game->screen_height - 80.0f;
                game->player->active = 1;
                game->player->weapon_level = 1;
            }
        }
    }
}
