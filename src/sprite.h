#ifndef SPRITE_H
#define SPRITE_H

#include <SDL.h>

typedef struct {
    SDL_Texture* texture;
    int width;
    int height;
    int frame_width;
    int frame_height;
    int frame_count;
    int columns;
} Sprite;

Sprite* sprite_load(SDL_Renderer* renderer, const char* path, int frame_width, int frame_height);
void sprite_free(Sprite* sprite);
void sprite_draw(SDL_Renderer* renderer, Sprite* sprite, int frame, float x, float y, float scale, float rotation, SDL_RendererFlip flip);
void sprite_draw_tinted(SDL_Renderer* renderer, Sprite* sprite, int frame, float x, float y, float scale, float rotation, SDL_Color tint);

#endif
