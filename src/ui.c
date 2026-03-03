#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Procedural font rendering - we'll use a simple bitmap font approach
// Since we want no external files, we'll render text as simple shapes

int ui_init(UISystem* ui) {
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        ui->initialized = 0;
        return 0;
    }
    
    // Try to load a system font
    const char* font_paths[] = {
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/opt/homebrew/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        NULL
    };
    
    ui->font_small = NULL;
    ui->font_medium = NULL;
    ui->font_large = NULL;
    
    for (int i = 0; font_paths[i]; i++) {
        ui->font_small = TTF_OpenFont(font_paths[i], 14);
        if (ui->font_small) {
            ui->font_medium = TTF_OpenFont(font_paths[i], 24);
            ui->font_large = TTF_OpenFont(font_paths[i], 48);
            break;
        }
    }
    
    if (!ui->font_small) {
        fprintf(stderr, "Could not load any font\n");
        TTF_Quit();
        ui->initialized = 0;
        return 0;
    }
    
    ui->initialized = 1;
    return 1;
}

void ui_shutdown(UISystem* ui) {
    if (!ui->initialized) return;
    
    if (ui->font_small) TTF_CloseFont(ui->font_small);
    if (ui->font_medium) TTF_CloseFont(ui->font_medium);
    if (ui->font_large) TTF_CloseFont(ui->font_large);
    
    TTF_Quit();
    ui->initialized = 0;
}

SDL_Color ui_color(Uint8 r, Uint8 g, Uint8 b) {
    return (SDL_Color){r, g, b, 255};
}

void ui_draw_text(UISystem* ui, SDL_Renderer* renderer, const char* text, 
                  int x, int y, SDL_Color color, int size) {
    if (!ui->initialized) return;
    
    TTF_Font* font;
    switch (size) {
        case 0: font = ui->font_small; break;
        case 2: font = ui->font_large; break;
        case 1:
        default: font = ui->font_medium; break;
    }
    
    if (!font) return;
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void ui_draw_number(UISystem* ui, SDL_Renderer* renderer, int number, 
                    int x, int y, SDL_Color color, int size) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", number);
    ui_draw_text(ui, renderer, buffer, x, y, color, size);
}

void ui_draw_hud(UISystem* ui, SDL_Renderer* renderer, int score, int lives, 
                 int wave, int weapon_level, int screen_w) {
    if (!ui->initialized) return;
    
    SDL_Color white = ui_color(255, 255, 255);
    SDL_Color cyan = ui_color(100, 255, 255);
    SDL_Color yellow = ui_color(255, 255, 100);
    
    // Score - top left
    ui_draw_text(ui, renderer, "SCORE", 10, 10, white, 0);
    ui_draw_number(ui, renderer, score, 10, 28, cyan, 1);
    
    // Lives - top right
    char lives_text[32];
    snprintf(lives_text, sizeof(lives_text), "LIVES: %d", lives);
    ui_draw_text(ui, renderer, lives_text, screen_w - 100, 10, white, 0);
    
    // Wave - center top
    char wave_text[32];
    snprintf(wave_text, sizeof(wave_text), "WAVE %d", wave);
    int wave_w = 0;
    TTF_SizeText(ui->font_medium, wave_text, &wave_w, NULL);
    ui_draw_text(ui, renderer, wave_text, (screen_w - wave_w) / 2, 10, yellow, 1);
    
    // Weapon level - bottom left
    char weapon_text[32];
    snprintf(weapon_text, sizeof(weapon_text), "WEAPON LVL %d", weapon_level);
    ui_draw_text(ui, renderer, weapon_text, 10, 560, white, 0);
    
    // Weapon indicator bars
    int bar_width = 20 * weapon_level;
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    SDL_Rect weapon_bar = {10, 580, bar_width, 8};
    SDL_RenderFillRect(renderer, &weapon_bar);
}

void ui_draw_menu(UISystem* ui, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!ui->initialized) {
        // Fallback if no font
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect overlay = {0, 0, screen_w, screen_h};
        SDL_RenderFillRect(renderer, &overlay);
        return;
    }
    
    SDL_Color white = ui_color(255, 255, 255);
    SDL_Color cyan = ui_color(100, 255, 255);
    
    // Semi-transparent background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, screen_w, screen_h};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Title
    const char* title = "STAR DEFENDER";
    int title_w = 0;
    TTF_SizeText(ui->font_large, title, &title_w, NULL);
    ui_draw_text(ui, renderer, title, (screen_w - title_w) / 2, 150, cyan, 2);
    
    // Subtitle
    const char* subtitle = "Press ENTER to start";
    int sub_w = 0;
    TTF_SizeText(ui->font_medium, subtitle, &sub_w, NULL);
    ui_draw_text(ui, renderer, subtitle, (screen_w - sub_w) / 2, 300, white, 1);
    
    // Controls
    ui_draw_text(ui, renderer, "WASD / Arrows - Move", (screen_w - 200) / 2, 380, white, 0);
    ui_draw_text(ui, renderer, "SPACE - Shoot", (screen_w - 150) / 2, 405, white, 0);
    ui_draw_text(ui, renderer, "ESC - Pause", (screen_w - 120) / 2, 430, white, 0);
}

void ui_draw_gameover(UISystem* ui, SDL_Renderer* renderer, int score, int screen_w, int screen_h) {
    if (!ui->initialized) return;
    
    SDL_Color white = ui_color(255, 255, 255);
    SDL_Color red = ui_color(255, 100, 100);
    SDL_Color yellow = ui_color(255, 255, 100);
    
    // Background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, screen_w, screen_h};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Game Over
    const char* gameover = "GAME OVER";
    int go_w = 0;
    TTF_SizeText(ui->font_large, gameover, &go_w, NULL);
    ui_draw_text(ui, renderer, gameover, (screen_w - go_w) / 2, 150, red, 2);
    
    // Final score
    char score_text[64];
    snprintf(score_text, sizeof(score_text), "Final Score: %d", score);
    int score_w = 0;
    TTF_SizeText(ui->font_medium, score_text, &score_w, NULL);
    ui_draw_text(ui, renderer, score_text, (screen_w - score_w) / 2, 280, yellow, 1);
    
    // Restart hint
    const char* restart = "Press ENTER to restart";
    int res_w = 0;
    TTF_SizeText(ui->font_medium, restart, &res_w, NULL);
    ui_draw_text(ui, renderer, restart, (screen_w - res_w) / 2, 400, white, 1);
}

void ui_draw_pause(UISystem* ui, SDL_Renderer* renderer, int screen_w, int screen_h) {
    if (!ui->initialized) return;
    
    SDL_Color white = ui_color(255, 255, 255);
    SDL_Color yellow = ui_color(255, 255, 100);
    
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, screen_w, screen_h};
    SDL_RenderFillRect(renderer, &overlay);
    
    // PAUSED text
    const char* paused = "PAUSED";
    int p_w = 0;
    TTF_SizeText(ui->font_large, paused, &p_w, NULL);
    ui_draw_text(ui, renderer, paused, (screen_w - p_w) / 2, 250, yellow, 2);
    
    // Resume hint
    const char* resume = "Press ESC to resume";
    int r_w = 0;
    TTF_SizeText(ui->font_medium, resume, &r_w, NULL);
    ui_draw_text(ui, renderer, resume, (screen_w - r_w) / 2, 350, white, 1);
}
