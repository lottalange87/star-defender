#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
    TTF_Font* font_small;
    TTF_Font* font_medium;
    TTF_Font* font_large;
    int initialized;
} UISystem;

int ui_init(UISystem* ui);
void ui_shutdown(UISystem* ui);

// Drawing functions
void ui_draw_text(UISystem* ui, SDL_Renderer* renderer, const char* text, 
                  int x, int y, SDL_Color color, int size);
void ui_draw_number(UISystem* ui, SDL_Renderer* renderer, int number, 
                    int x, int y, SDL_Color color, int size);

// HUD elements
void ui_draw_hud(UISystem* ui, SDL_Renderer* renderer, int score, int lives, 
                 int wave, int weapon_level, int screen_w);
void ui_draw_menu(UISystem* ui, SDL_Renderer* renderer, int screen_w, int screen_h);
void ui_draw_gameover(UISystem* ui, SDL_Renderer* renderer, int score, int screen_w, int screen_h);
void ui_draw_pause(UISystem* ui, SDL_Renderer* renderer, int screen_w, int screen_h);

// Helper
SDL_Color ui_color(Uint8 r, Uint8 g, Uint8 b);

#endif
