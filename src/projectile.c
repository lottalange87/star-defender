#include "projectile.h"
#include <math.h>
#include <stdlib.h>

void projectile_system_init(ProjectileSystem* ps) {
    for (int i = 0; i < MAX_PROJECTILE_VISUALS; i++) {
        ps->visuals[i].active = 0;
    }
}

void projectile_spawn_visual(ProjectileSystem* ps, Vec2 pos, Vec2 vel, int type) {
    for (int i = 0; i < MAX_PROJECTILE_VISUALS; i++) {
        if (!ps->visuals[i].active) {
            ps->visuals[i].pos = pos;
            ps->visuals[i].vel = vel;
            ps->visuals[i].rotation = 0;
            ps->visuals[i].rot_speed = (type == 0) ? 10.0f : -8.0f;
            ps->visuals[i].type = type;
            ps->visuals[i].active = 1;
            ps->visuals[i].life = 0;
            ps->visuals[i].pulse = 0;
            return;
        }
    }
}

void projectile_update_visuals(ProjectileSystem* ps, float dt) {
    for (int i = 0; i < MAX_PROJECTILE_VISUALS; i++) {
        if (ps->visuals[i].active) {
            ps->visuals[i].pos.x += ps->visuals[i].vel.x * dt;
            ps->visuals[i].pos.y += ps->visuals[i].vel.y * dt;
            ps->visuals[i].rotation += ps->visuals[i].rot_speed * dt;
            ps->visuals[i].life += dt;
            ps->visuals[i].pulse = sinf(ps->visuals[i].life * 15.0f) * 0.5f + 0.5f;
            
            // Deactivate if off screen
            if (ps->visuals[i].pos.y < -50 || ps->visuals[i].pos.y > 700 ||
                ps->visuals[i].pos.x < -50 || ps->visuals[i].pos.x > 850) {
                ps->visuals[i].active = 0;
            }
        }
    }
}

static void draw_rotated_rect(SDL_Renderer* renderer, float cx, float cy, 
                               float w, float h, float angle, 
                               Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    float rad = angle * 3.14159f / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);
    
    float hw = w / 2.0f;
    float hh = h / 2.0f;
    
    // Four corners relative to center
    float corners[4][2] = {
        {-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}
    };
    
    SDL_Vertex verts[4];
    for (int i = 0; i < 4; i++) {
        float rx = corners[i][0] * cos_a - corners[i][1] * sin_a;
        float ry = corners[i][0] * sin_a + corners[i][1] * cos_a;
        verts[i].position.x = cx + rx;
        verts[i].position.y = cy + ry;
        verts[i].color.r = r;
        verts[i].color.g = g;
        verts[i].color.b = b;
        verts[i].color.a = a;
    }
    
    int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
}

static void draw_energy_core(SDL_Renderer* renderer, float x, float y, int type, float pulse) {
    // Inner bright core
    float size = 4.0f + pulse * 2.0f;
    
    Uint8 r1, g1, b1, r2, g2, b2;
    if (type == 0) { // Player - cyan/blue
        r1 = 200; g1 = 255; b1 = 255;
        r2 = 100; g2 = 200; b2 = 255;
    } else { // Enemy - red/orange
        r1 = 255; g1 = 255; b1 = 200;
        r2 = 255; g2 = 150; b2 = 100;
    }
    
    // Outer glow
    for (int i = 3; i >= 0; i--) {
        float s = size + i * 3.0f;
        Uint8 alpha = 80 - i * 20;
        SDL_SetRenderDrawColor(renderer, r2, g2, b2, alpha);
        SDL_Rect glow = {(int)(x - s), (int)(y - s), (int)(s * 2), (int)(s * 2)};
        SDL_RenderFillRect(renderer, &glow);
    }
    
    // Core
    SDL_SetRenderDrawColor(renderer, r1, g1, b1, 255);
    SDL_Rect core = {(int)(x - size/2), (int)(y - size/2), (int)size, (int)size};
    SDL_RenderFillRect(renderer, &core);
}

static void draw_energy_ring(SDL_Renderer* renderer, float x, float y, float rotation, 
                              int type, float pulse) {
    float rad = rotation * 3.14159f / 180.0f;
    
    Uint8 r, g, b;
    if (type == 0) { // Player
        r = 100; g = 220; b = 255;
    } else {
        r = 255; g = 120; b = 80;
    }
    
    // Draw spinning energy blades
    int blades = 3;
    for (int i = 0; i < blades; i++) {
        float angle = rad + (i * 6.28318f / blades);
        float len = 12.0f + pulse * 4.0f;
        float width = 3.0f;
        
        float x1 = x + cosf(angle) * 4.0f;
        float y1 = y + sinf(angle) * 4.0f;
        float x2 = x + cosf(angle) * len;
        float y2 = y + sinf(angle) * len;
        
        // Draw blade as line with thickness
        for (int t = -1; t <= 1; t++) {
            float perp_x = -sinf(angle) * t * width;
            float perp_y = cosf(angle) * t * width;
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 200 - abs(t) * 50);
            SDL_RenderDrawLine(renderer, 
                (int)(x1 + perp_x), (int)(y1 + perp_y),
                (int)(x2 + perp_x), (int)(y2 + perp_y));
        }
    }
}

static void draw_energy_trail(SDL_Renderer* renderer, float x, float y, float vy, int type) {
    int segments = 5;
    float trail_length = 20.0f;
    
    Uint8 r, g, b;
    if (type == 0) { // Player - cyan
        r = 50; g = 200; b = 255;
    } else { // Enemy - red
        r = 255; g = 80; b = 50;
    }
    
    float dir = (vy < 0) ? 1.0f : -1.0f; // Trail goes opposite to movement
    
    for (int i = 0; i < segments; i++) {
        float t = (float)i / segments;
        float ty = y + dir * t * trail_length;
        float size = 6.0f * (1.0f - t);
        Uint8 alpha = (Uint8)(200 * (1.0f - t));
        
        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
        SDL_Rect seg = {(int)(x - size/2), (int)(ty - 2), (int)size, 4};
        SDL_RenderFillRect(renderer, &seg);
    }
}

void draw_fancy_projectile(SDL_Renderer* renderer, float x, float y, float rotation, int type, float pulse) {
    // Determine direction for trail
    float vy = (type == 0) ? -1.0f : 1.0f;
    
    // Draw trail first (behind)
    draw_energy_trail(renderer, x, y, vy, type);
    
    // Draw spinning ring
    draw_energy_ring(renderer, x, y, rotation, type, pulse);
    
    // Draw core (on top)
    draw_energy_core(renderer, x, y, type, pulse);
}

void projectile_draw_visuals(ProjectileSystem* ps, SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_PROJECTILE_VISUALS; i++) {
        if (ps->visuals[i].active) {
            draw_fancy_projectile(renderer, 
                ps->visuals[i].pos.x, 
                ps->visuals[i].pos.y,
                ps->visuals[i].rotation,
                ps->visuals[i].type,
                ps->visuals[i].pulse);
        }
    }
}
