#ifndef MENU_H
#define MENU_H

#include <SDL.h>
#include "background.h"

typedef enum {
    MENU_ITEM_SINGLE_PLAYER,
    MENU_ITEM_TWO_PLAYER,
    MENU_ITEM_HIGHSCORES,
    MENU_ITEM_SETTINGS,
    MENU_ITEM_QUIT,
    NUM_MENU_ITEMS
} MenuItem;

typedef struct {
    MenuItem selected;
    int active;
    float transition_alpha;
    Background bg;
    float time;
} MenuSystem;

void menu_init(MenuSystem* menu, int screen_w, int screen_h);
void menu_update(MenuSystem* menu, float dt);
void menu_draw(MenuSystem* menu, SDL_Renderer* renderer, int screen_w, int screen_h);

// Input handling
void menu_move_up(MenuSystem* menu);
void menu_move_down(MenuSystem* menu);
MenuItem menu_select(MenuSystem* menu);

// Draw helpers
void menu_draw_title(SDL_Renderer* renderer, int screen_w, int y);
void menu_draw_item(SDL_Renderer* renderer, const char* text, int x, int y, int selected, float time);
void menu_draw_stars(SDL_Renderer* renderer, float time, int screen_w, int screen_h);

#endif
