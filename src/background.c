#include "background.h"
#include <stdlib.h>
#include <math.h>

// Helper to set pixel with alpha blending
static void set_pixel_rgba(SDL_Surface* surf, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) return;
    Uint32* pixels = (Uint32*)surf->pixels;
    
    // Get existing pixel
    Uint32 existing = pixels[y * surf->w + x];
    Uint8 er, eg, eb, ea;
    SDL_GetRGBA(existing, surf->format, &er, &eg, &eb, &ea);
    
    // Alpha blend
    float alpha = a / 255.0f;
    Uint8 nr = (Uint8)(r * alpha + er * (1.0f - alpha));
    Uint8 ng = (Uint8)(g * alpha + eg * (1.0f - alpha));
    Uint8 nb = (Uint8)(b * alpha + eb * (1.0f - alpha));
    
    pixels[y * surf->w + x] = SDL_MapRGBA(surf->format, nr, ng, nb, 255);
}

void generate_nebula(Nebula* n, int screen_w, int screen_h) {
    n->x = (float)(rand() % screen_w);
    n->y = (float)(rand() % screen_h);
    n->radius = 100.0f + (rand() % 200);
    
    // Random nebula colors (purple, blue, pink)
    int color_type = rand() % 3;
    switch (color_type) {
        case 0: // Purple
            n->color1 = (SDL_Color){80, 20, 120, 255};
            n->color2 = (SDL_Color){40, 10, 80, 255};
            break;
        case 1: // Blue
            n->color1 = (SDL_Color){20, 60, 120, 255};
            n->color2 = (SDL_Color){10, 30, 60, 255};
            break;
        case 2: // Pink/Red
            n->color1 = (SDL_Color){120, 30, 60, 255};
            n->color2 = (SDL_Color){60, 15, 30, 255};
            break;
    }
    n->alpha = 0.3f + (rand() % 30) / 100.0f;
}

void generate_planet(Planet* p, int screen_w, int screen_h) {
    p->x = (float)(rand() % screen_w);
    p->y = (float)(rand() % screen_h);
    p->radius = 30.0f + (rand() % 50);
    p->atmosphere = 1.2f + (rand() % 20) / 100.0f;
    p->has_rings = (rand() % 3 == 0); // 33% chance for rings
    p->ring_angle = (rand() % 60) - 30.0f;
    
    // Planet colors
    int color_type = rand() % 5;
    switch (color_type) {
        case 0: p->color = (SDL_Color){200, 100, 50, 255}; break;  // Mars-like
        case 1: p->color = (SDL_Color){100, 150, 200, 255}; break; // Ice
        case 2: p->color = (SDL_Color){150, 200, 100, 255}; break; // Earth-like
        case 3: p->color = (SDL_Color){200, 180, 120, 255}; break; // Gas giant
        case 4: p->color = (SDL_Color){180, 100, 180, 255}; break; // Purple
    }
}

void background_init(Background* bg, int screen_w, int screen_h) {
    bg->offset_y = 0;
    bg->time = 0;
    
    // Initialize stars with different layers
    for (int i = 0; i < MAX_STARS; i++) {
        bg->stars[i].x = (float)(rand() % screen_w);
        bg->stars[i].y = (float)(rand() % screen_h);
        bg->stars[i].size = 1.0f + (rand() % 30) / 10.0f;
        
        // Parallax layers - closer stars move faster
        float layer = (rand() % 3) + 1; // 1, 2, or 3
        bg->stars[i].speed = 10.0f * layer;
        bg->stars[i].brightness = 0.5f + (rand() % 50) / 100.0f;
        
        // Star colors (mostly white, some blue/yellow tint)
        int tint = rand() % 10;
        if (tint < 7) {
            bg->stars[i].r = bg->stars[i].g = bg->stars[i].b = 255;
        } else if (tint < 9) {
            bg->stars[i].r = 200; bg->stars[i].g = 220; bg->stars[i].b = 255; // Blue-ish
        } else {
            bg->stars[i].r = 255; bg->stars[i].g = 240; bg->stars[i].b = 200; // Yellow-ish
        }
    }
    
    // Generate nebulae
    for (int i = 0; i < MAX_NEBULA; i++) {
        generate_nebula(&bg->nebulae[i], screen_w, screen_h);
    }
    
    // Generate planets
    for (int i = 0; i < MAX_PLANETS; i++) {
        generate_planet(&bg->planets[i], screen_w, screen_h);
    }
}

void background_update(Background* bg, float dt, int screen_w, int screen_h) {
    bg->time += dt;
    bg->offset_y += dt * 20.0f; // Slow drift
    
    // Update stars
    for (int i = 0; i < MAX_STARS; i++) {
        bg->stars[i].y += bg->stars[i].speed * dt;
        
        if (bg->stars[i].y > screen_h) {
            bg->stars[i].y = 0;
            bg->stars[i].x = (float)(rand() % screen_w);
        }
    }
    
    // Slowly move planets
    for (int i = 0; i < MAX_PLANETS; i++) {
        bg->planets[i].y += 2.0f * dt;
        if (bg->planets[i].y - bg->planets[i].radius > screen_h) {
            generate_planet(&bg->planets[i], screen_w, screen_h);
            bg->planets[i].y = -bg->planets[i].radius * 2;
        }
    }
}

void background_draw_nebula(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h) {
    (void)screen_h;
    
    for (int n = 0; n < MAX_NEBULA; n++) {
        Nebula* neb = &bg->nebulae[n];
        
        // Draw soft nebula as multiple circles
        int steps = 20;
        for (int i = steps; i >= 0; i--) {
            float t = (float)i / steps;
            float radius = neb->radius * (0.5f + t * 0.5f);
            
            Uint8 r = (Uint8)(neb->color1.r * (1-t) + neb->color2.r * t);
            Uint8 g = (Uint8)(neb->color1.g * (1-t) + neb->color2.g * t);
            Uint8 b = (Uint8)(neb->color1.b * (1-t) + neb->color2.b * t);
            Uint8 a = (Uint8)(neb->alpha * 30 * (1-t));
            
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            
            // Draw filled circle approximation
            for (int y = (int)(-radius); y <= (int)radius; y += 4) {
                for (int x = (int)(-radius); x <= (int)radius; x += 4) {
                    if (x*x + y*y <= radius*radius) {
                        SDL_Rect rect = {
                            (int)(neb->x + x),
                            (int)(neb->y + y),
                            4, 4
                        };
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
        }
    }
}

void background_draw_planets(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h) {
    (void)screen_w;
    (void)screen_h;
    
    for (int p = 0; p < MAX_PLANETS; p++) {
        Planet* planet = &bg->planets[p];
        
        // Atmosphere glow
        float atmos_r = planet->radius * planet->atmosphere;
        for (int i = 3; i >= 0; i--) {
            float r = atmos_r + i * 10;
            Uint8 alpha = 40 - i * 10;
            SDL_SetRenderDrawColor(renderer, planet->color.r, planet->color.g, planet->color.b, alpha);
            
            for (int y = (int)(-r); y <= (int)r; y += 6) {
                for (int x = (int)(-r); x <= (int)r; x += 6) {
                    if (x*x + y*y <= r*r) {
                        SDL_Rect rect = {
                            (int)(planet->x + x),
                            (int)(planet->y + y),
                            6, 6
                        };
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
        }
        
        // Planet body with shading
        for (int y = (int)(-planet->radius); y <= (int)planet->radius; y += 2) {
            for (int x = (int)(-planet->radius); x <= (int)planet->radius; x += 2) {
                float dist = sqrtf(x*x + y*y);
                if (dist <= planet->radius) {
                    // Simple shading based on position
                    float shade = 1.0f - (x / planet->radius) * 0.4f;
                    Uint8 r = (Uint8)(planet->color.r * shade);
                    Uint8 g = (Uint8)(planet->color.g * shade);
                    Uint8 b = (Uint8)(planet->color.b * shade);
                    
                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                    SDL_Rect rect = {
                        (int)(planet->x + x),
                        (int)(planet->y + y),
                        2, 2
                    };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
        
        // Rings (if applicable)
        if (planet->has_rings) {
            float ring_inner = planet->radius * 1.3f;
            float ring_outer = planet->radius * 2.0f;
            
            for (float r = ring_inner; r <= ring_outer; r += 3) {
                float alpha = 1.0f - (r - ring_inner) / (ring_outer - ring_inner);
                Uint8 a = (Uint8)(150 * alpha);
                SDL_SetRenderDrawColor(renderer, 200, 180, 150, a);
                
                // Draw ring ellipse
                for (float angle = 0; angle < 6.283f; angle += 0.1f) {
                    float rx = cosf(angle) * r;
                    float ry = sinf(angle) * r * 0.3f; // Flattened
                    
                    // Rotate
                    float rad = planet->ring_angle * 3.14159f / 180.0f;
                    float rxr = rx * cosf(rad) - ry * sinf(rad);
                    float ryr = rx * sinf(rad) + ry * cosf(rad);
                    
                    SDL_Rect rect = {
                        (int)(planet->x + rxr),
                        (int)(planet->y + ryr),
                        3, 3
                    };
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
}

void background_draw_starfield(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h) {
    (void)screen_w;
    (void)screen_h;
    
    for (int i = 0; i < MAX_STARS; i++) {
        Star* star = &bg->stars[i];
        
        Uint8 alpha = (Uint8)(star->brightness * 255);
        SDL_SetRenderDrawColor(renderer, star->r, star->g, star->b, alpha);
        
        // Draw star with glow for larger ones
        if (star->size > 2.0f) {
            // Glow
            SDL_SetRenderDrawColor(renderer, star->r, star->g, star->b, alpha / 3);
            SDL_Rect glow = {
                (int)(star->x - star->size),
                (int)(star->y - star->size),
                (int)(star->size * 2),
                (int)(star->size * 2)
            };
            SDL_RenderFillRect(renderer, &glow);
        }
        
        // Core
        SDL_SetRenderDrawColor(renderer, star->r, star->g, star->b, alpha);
        SDL_Rect core = {
            (int)(star->x - star->size/2),
            (int)(star->y - star->size/2),
            (int)star->size,
            (int)star->size
        };
        SDL_RenderFillRect(renderer, &core);
    }
}

void background_draw(Background* bg, SDL_Renderer* renderer, int screen_w, int screen_h) {
    // Clear with deep space color
    SDL_SetRenderDrawColor(renderer, 5, 5, 15, 255);
    SDL_RenderClear(renderer);
    
    // Enable alpha blending for nebula
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Draw layers from back to front
    background_draw_nebula(bg, renderer, screen_w, screen_h);
    background_draw_planets(bg, renderer, screen_w, screen_h);
    background_draw_starfield(bg, renderer, screen_w, screen_h);
    
    // Reset blend mode
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
