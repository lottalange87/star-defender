#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "particle.h"
#include "explosion.h"
#include <SDL.h>

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_PAUSE
} GameState;

typedef struct {
    GameState state;
    int score;
    int lives;
    int wave;
    float wave_timer;
    float spawn_timer;
    int screen_width;
    int screen_height;
    
    EntityManager entities;
    ParticleSystem particles;
    ExplosionSystem explosions;
    Entity* player;
    
    // Sprites
    Sprite* sprite_player;
    Sprite* sprite_enemy_normal;
    Sprite* sprite_enemy_fast;
    Sprite* sprite_enemy_tank;
    Sprite* sprite_projectile_player;
    Sprite* sprite_projectile_enemy;
    Sprite* sprite_powerup;
    
    // Input
    int key_left, key_right, key_up, key_down, key_shoot;
    float shoot_cooldown;
    
    // Background stars
    struct { float x, y, speed; int size; } stars[100];
} Game;

int game_init(Game* game, int screen_w, int screen_h, SDL_Renderer* renderer);
void game_shutdown(Game* game);
void game_reset(Game* game);
void game_handle_input(Game* game, SDL_Event* event);
void game_update(Game* game, float dt);
void game_draw(Game* game, SDL_Renderer* renderer);
void game_check_collisions(Game* game);

#endif
