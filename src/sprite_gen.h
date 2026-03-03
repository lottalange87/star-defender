#ifndef SPRITE_GEN_H
#define SPRITE_GEN_H

#include <SDL.h>

// Generate procedural pixel-art sprites
SDL_Surface* gen_player_ship(void);
SDL_Surface* gen_enemy_normal(void);
SDL_Surface* gen_enemy_fast(void);
SDL_Surface* gen_enemy_tank(void);
SDL_Surface* gen_projectile_player(void);
SDL_Surface* gen_projectile_enemy(void);
SDL_Surface* gen_powerup(void);

// Explosion animation frames
#define EXPLOSION_FRAMES 8
SDL_Surface* gen_explosion_frame(int frame);

// Thruster flame
SDL_Surface* gen_thruster(void);

#endif
