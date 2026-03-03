#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL.h>

#define MAX_STARS 200
#define MAX_NEBULA 5
#define MAX_PLANETS 3

typedef struct {
    float x, y;
    float size;
    float speed;
    float brightness;
    Uint8 r, g, b;
} Star;

typedef struct {
    float x, y;
    float radius;
    SDL_Color color1;
    SDL_Color color2;
    float alpha;
} Nebula;

typedef struct {
    float x, y;
    float radius;
    SDL_Color color;
    float atmosphere;
    int has_rings;
    float ring_angle;
} Planet;

typedef struct {
    Star stars[MAX_STARS];
    Nebula nebulae[MAX_NEBULA];
    Planet planets[MAX_PLANETS];
    float offset_y;
    float time;
} Background;

void background_init(Background* bg, int screen_w, int screen_h);
void background_update(Background* bg, float dt, int screen_w, int screen_h);
void background_draw(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h);

// Draw individual elements
void background_draw_starfield(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h);
void background_draw_nebula(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h);
void background_draw_planets(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h);

// Procedural generation helpers
void generate_nebula(Nebula* n, int screen_w, int screen_h);
void generate_planet(Planet* p, int screen_w, int screen_h);

#endif
