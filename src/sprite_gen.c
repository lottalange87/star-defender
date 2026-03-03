#include "sprite_gen.h"
#include <stdlib.h>
#include <math.h>

// Helper to set pixel
static void set_pixel(SDL_Surface* surf, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) return;
    Uint32* pixels = (Uint32*)surf->pixels;
    Uint32 color = SDL_MapRGBA(surf->format, r, g, b, a);
    pixels[y * surf->w + x] = color;
}

// Draw filled circle
static void draw_circle(SDL_Surface* surf, int cx, int cy, int r, Uint8 red, Uint8 g, Uint8 b, Uint8 a) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                set_pixel(surf, cx + x, cy + y, red, g, b, a);
            }
        }
    }
}

// Draw line
static void draw_line(SDL_Surface* surf, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        set_pixel(surf, x1, y1, r, g, b, a);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

// Draw filled triangle
static void draw_triangle(SDL_Surface* surf, int x1, int y1, int x2, int y2, int x3, int y3, 
                          Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    // Simple scanline fill
    int min_y = y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);
    int max_y = y1 > y2 ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3);
    
    for (int y = min_y; y <= max_y; y++) {
        int x_start = 9999, x_end = -9999;
        
        // Check each edge
        struct { int x1, y1, x2, y2; } edges[] = {{x1,y1,x2,y2}, {x2,y2,x3,y3}, {x3,y3,x1,y1}};
        for (int e = 0; e < 3; e++) {
            int ey1 = edges[e].y1, ey2 = edges[e].y2;
            if ((y >= ey1 && y < ey2) || (y >= ey2 && y < ey1)) {
                float t = (float)(y - ey1) / (ey2 - ey1);
                int x = edges[e].x1 + (int)(t * (edges[e].x2 - edges[e].x1));
                if (x < x_start) x_start = x;
                if (x > x_end) x_end = x;
            }
        }
        
        for (int x = x_start; x <= x_end; x++) {
            set_pixel(surf, x, y, r, g, b, a);
        }
    }
}

SDL_Surface* gen_player_ship(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 32, 32, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Main body - sleek fighter
    // Cockpit
    draw_triangle(surf, 16, 4, 12, 14, 20, 14, 100, 200, 255, 255);
    // Body
    draw_triangle(surf, 16, 2, 8, 28, 16, 24, 180, 200, 220, 255);
    draw_triangle(surf, 16, 2, 24, 28, 16, 24, 160, 180, 200, 255);
    // Wings
    draw_triangle(surf, 8, 16, 2, 26, 8, 28, 120, 140, 160, 255);
    draw_triangle(surf, 24, 16, 30, 26, 24, 28, 100, 120, 140, 255);
    // Engine glow
    draw_circle(surf, 16, 28, 3, 100, 200, 255, 200);
    
    return surf;
}

SDL_Surface* gen_enemy_normal(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 28, 28, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Classic invader style - red
    // Body
    draw_triangle(surf, 14, 24, 6, 8, 22, 8, 220, 60, 60, 255);
    // Cockpit
    draw_circle(surf, 14, 14, 5, 180, 40, 40, 255);
    // Wing details
    draw_line(surf, 6, 12, 6, 20, 180, 40, 40, 255);
    draw_line(surf, 22, 12, 22, 20, 180, 40, 40, 255);
    
    return surf;
}

SDL_Surface* gen_enemy_fast(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 24, 24, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Sleek dart - magenta
    draw_triangle(surf, 12, 22, 4, 6, 20, 6, 220, 60, 220, 255);
    // Sharp nose
    draw_triangle(surf, 12, 2, 10, 8, 14, 8, 255, 100, 255, 255);
    // Engine trails
    draw_circle(surf, 12, 20, 2, 255, 100, 255, 180);
    
    return surf;
}

SDL_Surface* gen_enemy_tank(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 40, 40, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Heavy bomber - yellow/orange
    // Main hull
    draw_triangle(surf, 20, 35, 8, 10, 32, 10, 220, 180, 40, 255);
    // Armor plates
    draw_triangle(surf, 20, 25, 12, 12, 28, 12, 200, 160, 30, 255);
    // Side pods
    draw_circle(surf, 10, 20, 6, 180, 140, 20, 255);
    draw_circle(surf, 30, 20, 6, 180, 140, 20, 255);
    // Heavy engines
    draw_circle(surf, 16, 36, 4, 255, 150, 50, 200);
    draw_circle(surf, 24, 36, 4, 255, 150, 50, 200);
    
    return surf;
}

SDL_Surface* gen_projectile_player(void) {
    // Larger projectile with glow effect
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 16, 24, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    int cx = 8, cy = 12;
    
    // Outer glow (cyan/blue)
    for (int r = 6; r >= 4; r--) {
        Uint8 alpha = 60 - (6-r) * 15;
        draw_circle(surf, cx, cy, r, 50, 150, 255, alpha);
    }
    
    // Middle glow
    draw_circle(surf, cx, cy, 4, 100, 200, 255, 180);
    
    // Core - bright white/cyan
    draw_circle(surf, cx, cy, 2, 200, 255, 255, 255);
    
    // Energy trail (pointed top and bottom)
    for (int y = 0; y < 24; y++) {
        float dist_from_center = fabsf(y - cy) / 12.0f;
        int width = (int)(3 * (1.0f - dist_from_center));
        if (width < 1) width = 1;
        
        Uint8 intensity = (Uint8)(200 * (1.0f - dist_from_center * 0.5f));
        Uint8 alpha = (Uint8)(255 * (1.0f - dist_from_center * 0.7f));
        
        for (int x = cx - width; x <= cx + width; x++) {
            // Blend with existing pixels
            set_pixel(surf, x, y, intensity, 255, 255, alpha);
        }
    }
    
    // Bright center line
    draw_line(surf, cx, 2, cx, 22, 255, 255, 255, 200);
    
    return surf;
}

SDL_Surface* gen_projectile_enemy(void) {
    // Larger enemy projectile - red/orange plasma
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 16, 24, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    int cx = 8, cy = 12;
    
    // Outer glow (orange/red)
    for (int r = 6; r >= 4; r--) {
        Uint8 alpha = 60 - (6-r) * 15;
        draw_circle(surf, cx, cy, r, 255, 100, 50, alpha);
    }
    
    // Middle glow
    draw_circle(surf, cx, cy, 4, 255, 150, 80, 180);
    
    // Core - bright white/orange
    draw_circle(surf, cx, cy, 2, 255, 220, 200, 255);
    
    // Energy trail
    for (int y = 0; y < 24; y++) {
        float dist_from_center = fabsf(y - cy) / 12.0f;
        int width = (int)(3 * (1.0f - dist_from_center));
        if (width < 1) width = 1;
        
        Uint8 intensity = (Uint8)(255 * (1.0f - dist_from_center * 0.5f));
        Uint8 green = (Uint8)(150 * (1.0f - dist_from_center));
        Uint8 alpha = (Uint8)(255 * (1.0f - dist_from_center * 0.7f));
        
        for (int x = cx - width; x <= cx + width; x++) {
            set_pixel(surf, x, y, intensity, green, 50, alpha);
        }
    }
    
    // Bright center line
    draw_line(surf, cx, 2, cx, 22, 255, 255, 200, 200);
    
    return surf;
}

SDL_Surface* gen_powerup(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 20, 20, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Green glowing orb
    draw_circle(surf, 10, 10, 8, 100, 255, 100, 255);
    draw_circle(surf, 10, 10, 5, 150, 255, 150, 255);
    draw_circle(surf, 10, 10, 2, 200, 255, 200, 255);
    
    return surf;
}

SDL_Surface* gen_explosion_frame(int frame) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 48, 48, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Expanding explosion
    float progress = (float)frame / EXPLOSION_FRAMES;
    int max_radius = 20;
    int radius = (int)(progress * max_radius);
    
    // Core - white/yellow
    draw_circle(surf, 24, 24, radius, 255, 255, 200, 255);
    // Middle - orange
    draw_circle(surf, 24, 24, (int)(radius * 0.8f), 255, 180, 50, 220);
    // Outer - red
    draw_circle(surf, 24, 24, (int)(radius * 0.5f), 255, 80, 30, 180);
    
    // Random sparks
    srand(frame * 12345);
    for (int i = 0; i < 8; i++) {
        float angle = (float)i / 8.0f * 6.28318f;
        int dist = radius + (rand() % 8);
        int x = 24 + (int)(cosf(angle) * dist);
        int y = 24 + (int)(sinf(angle) * dist);
        int size = 2 + (rand() % 3);
        draw_circle(surf, x, y, size, 255, 150 + (rand() % 100), 50, 255 - (int)(progress * 100));
    }
    
    return surf;
}

SDL_Surface* gen_thruster(void) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 8, 12, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
    
    // Flame gradient
    for (int y = 0; y < 12; y++) {
        float t = (float)y / 12.0f;
        Uint8 r = 255;
        Uint8 g = (Uint8)(200 * (1.0f - t));
        Uint8 b = (Uint8)(100 * (1.0f - t));
        Uint8 a = (Uint8)(255 * (1.0f - t * 0.5f));
        
        int width = (int)(4 * (1.0f - t));
        for (int x = 4 - width; x < 4 + width; x++) {
            set_pixel(surf, x, y, r, g, b, a);
        }
    }
    
    return surf;
}
