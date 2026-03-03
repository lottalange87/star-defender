#include "sprite.h"
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

Sprite* sprite_load(SDL_Renderer* renderer, const char* path, int frame_width, int frame_height) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        fprintf(stderr, "Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }
    
    Sprite* sprite = malloc(sizeof(Sprite));
    sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
    sprite->width = surface->w;
    sprite->height = surface->h;
    
    if (frame_width > 0 && frame_height > 0) {
        sprite->frame_width = frame_width;
        sprite->frame_height = frame_height;
        sprite->columns = surface->w / frame_width;
        sprite->frame_count = (surface->w / frame_width) * (surface->h / frame_height);
    } else {
        sprite->frame_width = surface->w;
        sprite->frame_height = surface->h;
        sprite->columns = 1;
        sprite->frame_count = 1;
    }
    
    SDL_FreeSurface(surface);
    return sprite;
}

Sprite* sprite_from_surface(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return NULL;
    
    Sprite* sprite = malloc(sizeof(Sprite));
    sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
    sprite->width = surface->w;
    sprite->height = surface->h;
    sprite->frame_width = surface->w;
    sprite->frame_height = surface->h;
    sprite->columns = 1;
    sprite->frame_count = 1;
    
    return sprite;
}

void sprite_free(Sprite* sprite) {
    if (sprite) {
        SDL_DestroyTexture(sprite->texture);
        free(sprite);
    }
}

void sprite_draw(SDL_Renderer* renderer, Sprite* sprite, int frame, float x, float y, float scale, float rotation, SDL_RendererFlip flip) {
    if (!sprite) return;
    
    int col = frame % sprite->columns;
    int row = frame / sprite->columns;
    
    SDL_Rect src = {
        col * sprite->frame_width,
        row * sprite->frame_height,
        sprite->frame_width,
        sprite->frame_height
    };
    
    SDL_Rect dst = {
        (int)(x - (sprite->frame_width * scale) / 2),
        (int)(y - (sprite->frame_height * scale) / 2),
        (int)(sprite->frame_width * scale),
        (int)(sprite->frame_height * scale)
    };
    
    SDL_RenderCopyEx(renderer, sprite->texture, &src, &dst, rotation, NULL, flip);
}

void sprite_draw_tinted(SDL_Renderer* renderer, Sprite* sprite, int frame, float x, float y, float scale, float rotation, SDL_Color tint) {
    if (!sprite) return;
    
    SDL_SetTextureColorMod(sprite->texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(sprite->texture, tint.a);
    
    sprite_draw(renderer, sprite, frame, x, y, scale, rotation, SDL_FLIP_NONE);
    
    SDL_SetTextureColorMod(sprite->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(sprite->texture, 255);
}
