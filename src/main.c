#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TARGET_FPS 60.0f
#define FRAME_TIME (1.0f / TARGET_FPS)

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow(
        "Star Defender",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    Game game;
    if (!game_init(&game, SCREEN_WIDTH, SCREEN_HEIGHT, renderer)) {
        fprintf(stderr, "Game init failed\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    int running = 1;
    Uint32 last_time = SDL_GetTicks();
    float accumulator = 0.0f;
    
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        
        if (dt > 0.1f) dt = 0.1f;
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                game_handle_input(&game, &event);
                if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    running = 0;
                }
            }
        }
        
        accumulator += dt;
        while (accumulator >= FRAME_TIME) {
            game_update(&game, FRAME_TIME);
            accumulator -= FRAME_TIME;
        }
        
        game_draw(&game, renderer);
        SDL_RenderPresent(renderer);
    }
    
    game_shutdown(&game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
