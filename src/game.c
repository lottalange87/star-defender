#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int game_init(Game* game, int screen_w, int screen_h) {
    game->screen_width = screen_w;
    game->screen_height = screen_h;
    game->state = STATE_MENU;
    
    entity_manager_init(&game->entities);
    particle_system_init(&game->particles);
    
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
    
    game->player = entity_spawn(&game->entities, ENTITY_PLAYER, 
                                 game->screen_width / 2.0f, 
                                 game->screen_height - 80.0f);
}

void game_handle_input(Game* game, SDL_Event* event) {
    int down = (event->type == SDL_KEYDOWN);
    
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
            if (down && game->state == STATE_MENU) {
                game->state = STATE_PLAYING;
            } else if (down && game->state == STATE_GAMEOVER) {
                game_reset(game);
                game->state = STATE_PLAYING;
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
            
            particle_spawn_thruster(&game->particles, 
                                   vec2(game->player->pos.x, game->player->pos.y + 16),
                                   3.14159f / 2.0f);
        }
        
        game->shoot_cooldown -= dt;
        if (game->key_shoot && game->shoot_cooldown <= 0) {
            game->shoot_cooldown = 0.15f;
            
            switch (game->player->weapon_level) {
                case 1:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x, game->player->pos.y - 16, 0, -500.0f);
                    break;
                case 2:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x - 8, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x + 8, game->player->pos.y - 16, 0, -500.0f);
                    break;
                case 3:
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x, game->player->pos.y - 16, 0, -500.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x - 12, game->player->pos.y - 12, -50.0f, -480.0f);
                    projectile_spawn(&game->entities, ENTITY_PROJECTILE_PLAYER,
                                    game->player->pos.x + 12, game->player->pos.y - 12, 50.0f, -480.0f);
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
        if (roll < 60) {
            entity_spawn(&game->entities, ENTITY_ENEMY_NORMAL, x, -30);
        } else if (roll < 85) {
            entity_spawn(&game->entities, ENTITY_ENEMY_FAST, x, -30);
        } else {
            entity_spawn(&game->entities, ENTITY_ENEMY_TANK, x, -30);
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
    
    for (int i = 0; i < 100; i++) {
        game->stars[i].y += game->stars[i].speed * dt;
        if (game->stars[i].y > game->screen_height) {
            game->stars[i].y = 0;
            game->stars[i].x = rand() % game->screen_width;
        }
    }
    
    game_check_collisions(game);
}

void game_draw(Game* game, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 5, 5, 15, 255);
    SDL_RenderClear(renderer);
    
    for (int i = 0; i < 100; i++) {
        int alpha = 100 + (int)(game->stars[i].speed * 1.5f);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha > 255 ? 255 : alpha);
        SDL_Rect star = {(int)game->stars[i].x, (int)game->stars[i].y, 
                        game->stars[i].size, game->stars[i].size};
        SDL_RenderFillRect(renderer, &star);
    }
    
    entity_draw(&game->entities, renderer);
    projectile_draw(&game->entities, renderer);
    particle_draw(&game->particles, renderer);
    
    // HUD
    char hud[128];
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Score
    snprintf(hud, sizeof(hud), "SCORE: %d", game->score);
    // TODO: render text
    
    // Lives
    snprintf(hud, sizeof(hud), "LIVES: %d", game->lives);
    
    // Wave
    snprintf(hud, sizeof(hud), "WAVE: %d", game->wave);
    
    // Menu / Game Over screens
    if (game->state == STATE_MENU) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, game->screen_width, game->screen_height};
        SDL_RenderFillRect(renderer, &overlay);
    } else if (game->state == STATE_GAMEOVER) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, game->screen_width, game->screen_height};
        SDL_RenderFillRect(renderer, &overlay);
    } else if (game->state == STATE_PAUSE) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_Rect overlay = {0, 0, game->screen_width, game->screen_height};
        SDL_RenderFillRect(renderer, &overlay);
    }
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
                entity_take_damage(enemy, 1, &game->particles);
                
                if (!enemy->active) {
                    game->score += enemy->score_value;
                    
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
            entity_take_damage(game->player, 1, &game->particles);
            game->lives--;
            
            if (game->player->hp <= 0 && game->lives > 0) {
                // Respawn
                game->player->hp = 3;
                game->player->pos.x = game->screen_width / 2.0f;
                game->player->pos.y = game->screen_height - 80.0f;
                game->player->active = 1;
                game->player->weapon_level = 1;
            }
            break;
        }
    }
    
    // Enemies vs player (collision)
    for (int e = 0; e < MAX_ENTITIES; e++) {
        Entity* enemy = &game->entities.entities[e];
        if (!enemy->active || enemy->type < ENTITY_ENEMY_NORMAL || 
            enemy->type > ENTITY_ENEMY_TANK) continue;
        
        if (game->player && game->player->active && entity_check_collision(enemy, game->player)) {
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
