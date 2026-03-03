#include "transition.h"
#include <math.h>

void transition_init(Transition* t) {
    t->type = TRANSITION_NONE;
    t->progress = 0.0f;
    t->speed = 1.0f;
    t->active = 0;
    t->callback = NULL;
    t->callback_data = NULL;
}

void transition_start(Transition* t, TransitionType type, float duration) {
    t->type = type;
    t->progress = 0.0f;
    t->speed = 1.0f / duration;
    t->active = 1;
}

void transition_update(Transition* t, float dt) {
    if (!t->active) return;
    
    t->progress += t->speed * dt;
    
    if (t->progress >= 1.0f) {
        t->progress = 1.0f;
        t->active = 0;
        
        if (t->callback) {
            t->callback(t->callback_data);
        }
    }
}

int transition_is_complete(Transition* t) {
    return !t->active && t->progress >= 1.0f;
}

void transition_draw_fade(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h) {
    Uint8 alpha = (Uint8)(t->progress * 255);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
    SDL_Rect rect = {0, 0, screen_w, screen_h};
    SDL_RenderFillRect(renderer, &rect);
}

void transition_draw_wipe(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h) {
    int wipe_height = (int)(screen_h * t->progress);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect rect = {0, 0, screen_w, wipe_height};
    SDL_RenderFillRect(renderer, &rect);
}

void transition_draw_star_warp(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h) {
    // Starfield warp effect
    int num_stars = 100;
    float cx = screen_w / 2.0f;
    float cy = screen_h / 2.0f;
    
    // Clear with fade
    Uint8 alpha = (Uint8)(t->progress * 200);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
    SDL_Rect clear = {0, 0, screen_w, screen_h};
    SDL_RenderFillRect(renderer, &clear);
    
    // Draw streaking stars
    for (int i = 0; i < num_stars; i++) {
        float angle = (i * 6.28318f) / num_stars;
        float dist = 50.0f + t->progress * 500.0f * (1.0f + (i % 5) * 0.2f);
        
        float x1 = cx + cosf(angle) * dist * 0.5f;
        float y1 = cy + sinf(angle) * dist * 0.5f;
        float x2 = cx + cosf(angle) * dist;
        float y2 = cy + sinf(angle) * dist;
        
        Uint8 star_alpha = (Uint8)(255 * (1.0f - t->progress));
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, star_alpha);
        SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
    }
}

void transition_draw(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!t->active && t->progress <= 0) return;
    
    switch (t->type) {
        case TRANSITION_FADE_OUT:
        case TRANSITION_FADE_IN:
            transition_draw_fade(t, renderer, screen_w, screen_h);
            break;
        case TRANSITION_WIPE:
            transition_draw_wipe(t, renderer, screen_w, screen_h);
            break;
        case TRANSITION_STAR_WARP:
            transition_draw_star_warp(t, renderer, screen_w, screen_h);
            break;
        default:
            break;
    }
}
