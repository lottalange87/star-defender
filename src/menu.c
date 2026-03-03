#include "menu.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void menu_init(MenuSystem* menu, int screen_w, int screen_h) {
    menu->selected = MENU_ITEM_SINGLE_PLAYER;
    menu->active = 1;
    menu->transition_alpha = 1.0f;
    menu->time = 0;
    background_init(&menu->bg, screen_w, screen_h);
}

void menu_update(MenuSystem* menu, float dt) {
    menu->time += dt;
    
    // Fade in
    if (menu->transition_alpha > 0) {
        menu->transition_alpha -= dt;
        if (menu->transition_alpha < 0) menu->transition_alpha = 0;
    }
}

void menu_move_up(MenuSystem* menu) {
    if (menu->selected > 0) {
        menu->selected--;
    } else {
        menu->selected = NUM_MENU_ITEMS - 1;
    }
}

void menu_move_down(MenuSystem* menu) {
    if (menu->selected < NUM_MENU_ITEMS - 1) {
        menu->selected++;
    } else {
        menu->selected = 0;
    }
}

MenuItem menu_select(MenuSystem* menu) {
    return menu->selected;
}

void menu_draw_title(SDL_Renderer* renderer, int screen_w, int y) {
    // Draw "STAR DEFENDER" with glow effect
    const char* title = "STAR DEFENDER";
    int title_len = strlen(title);
    int char_width = 24;
    int total_width = title_len * char_width;
    int start_x = (screen_w - total_width) / 2;
    
    // Glow layers
    for (int glow = 3; glow >= 0; glow--) {
        Uint8 alpha = 50 - glow * 12;
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, alpha);
        
        for (int i = 0; i < title_len; i++) {
            int x = start_x + i * char_width;
            int offset = glow * 4;
            SDL_Rect glow_rect = {x - offset, y - offset, char_width + offset*2, 40 + offset*2};
            SDL_RenderFillRect(renderer, &glow_rect);
        }
    }
    
    // Main title - draw as block letters
    for (int i = 0; i < title_len; i++) {
        int x = start_x + i * char_width;
        
        // Letter shadow
        SDL_SetRenderDrawColor(renderer, 0, 50, 100, 255);
        SDL_Rect shadow = {x + 3, y + 3, char_width - 6, 34};
        SDL_RenderFillRect(renderer, &shadow);
        
        // Letter
        SDL_SetRenderDrawColor(renderer, 100, 220, 255, 255);
        SDL_Rect letter = {x, y, char_width - 6, 34};
        SDL_RenderFillRect(renderer, &letter);
        
        // Highlight
        SDL_SetRenderDrawColor(renderer, 200, 255, 255, 255);
        SDL_Rect highlight = {x, y, char_width - 6, 6};
        SDL_RenderFillRect(renderer, &highlight);
    }
}

void menu_draw_item(SDL_Renderer* renderer, const char* text, int x, int y, int selected, float time) {
    int len = strlen(text);
    int char_w = 16;
    int total_w = len * char_w;
    int start_x = x - total_w / 2;
    
    // Selection indicator
    if (selected) {
        float pulse = (sinf(time * 8.0f) + 1.0f) / 2.0f;
        Uint8 alpha = (Uint8)(150 + pulse * 105);
        
        // Glow
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, alpha / 2);
        SDL_Rect glow = {start_x - 20, y - 5, total_w + 40, 30};
        SDL_RenderFillRect(renderer, &glow);
        
        // Arrow left
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 8; i++) {
            SDL_RenderDrawLine(renderer, start_x - 15, y + 10, start_x - 5 - i, y + 10 - i);
            SDL_RenderDrawLine(renderer, start_x - 15, y + 10, start_x - 5 - i, y + 10 + i);
        }
        
        // Arrow right
        for (int i = 0; i < 8; i++) {
            SDL_RenderDrawLine(renderer, start_x + total_w + 15, y + 10, start_x + total_w + 5 + i, y + 10 - i);
            SDL_RenderDrawLine(renderer, start_x + total_w + 15, y + 10, start_x + total_w + 5 + i, y + 10 + i);
        }
    }
    
    // Text color
    if (selected) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 180, 180, 200, 255);
    }
    
    // Draw text as simple blocks (placeholder for real font)
    for (int i = 0; i < len; i++) {
        int cx = start_x + i * char_w;
        SDL_Rect char_rect = {cx, y, char_w - 4, 20};
        SDL_RenderFillRect(renderer, &char_rect);
    }
}

void menu_draw_stars(SDL_Renderer* renderer, float time, int screen_w, int screen_h) {
    // Animated stars in background
    for (int i = 0; i < 100; i++) {
        float x = (i * 37) % screen_w;
        float y = ((i * 23) + (int)(time * 20)) % screen_h;
        float size = 1.0f + (i % 3);
        float twinkle = (sinf(time * 3 + i) + 1.0f) / 2.0f;
        
        Uint8 alpha = (Uint8)(100 + twinkle * 155);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
        
        SDL_Rect star = {(int)x, (int)y, (int)size, (int)size};
        SDL_RenderFillRect(renderer, &star);
    }
}

void menu_draw(MenuSystem* menu, SDL_Renderer* renderer, int screen_w, int screen_h) {
    // Draw animated background
    background_draw(&menu->bg, renderer, screen_w, screen_h);
    background_update(&menu->bg, 0.016f, screen_w, screen_h); // Update at ~60fps
    
    // Reset blend mode after background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Draw title
    menu_draw_title(renderer, screen_w, 80);
    
    // Menu items
    const char* items[] = {
        "1 PLAYER",
        "2 PLAYERS",
        "HIGH SCORES",
        "SETTINGS",
        "QUIT"
    };
    
    int start_y = 250;
    int spacing = 50;
    
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
        int y = start_y + i * spacing;
        menu_draw_item(renderer, items[i], screen_w / 2, y, 
                      menu->selected == i, menu->time);
    }
    
    // Instructions at bottom
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    // Draw small text: "UP/DOWN to select, ENTER to confirm"
    
    // Fade overlay for transition
    if (menu->transition_alpha > 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)(menu->transition_alpha * 255));
        SDL_Rect overlay = {0, 0, screen_w, screen_h};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}
