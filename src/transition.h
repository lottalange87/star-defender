#ifndef TRANSITION_H
#define TRANSITION_H

#include <SDL.h>

typedef enum {
    TRANSITION_NONE,
    TRANSITION_FADE_OUT,
    TRANSITION_FADE_IN,
    TRANSITION_WIPE,
    TRANSITION_STAR_WARP
} TransitionType;

typedef struct {
    TransitionType type;
    float progress;      // 0.0 to 1.0
    float speed;
    int active;
    void (*callback)(void*);  // Called when transition completes
    void* callback_data;
} Transition;

void transition_init(Transition* t);
void transition_start(Transition* t, TransitionType type, float duration);
void transition_update(Transition* t, float dt);
void transition_draw(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h);
int transition_is_complete(Transition* t);

// Specific transition effects
void transition_draw_fade(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h);
void transition_draw_wipe(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h);
void transition_draw_star_warp(Transition* t, SDL_Renderer* renderer, int screen_w, int screen_h);

#endif
