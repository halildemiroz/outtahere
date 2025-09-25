#include "game.h"
#include "player.h"
#include "tilemap.h"
#include <stdio.h>
#include <string.h>

static Player player;
static Tilemap tilemap;

int game_init(Game *game) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }
    
    // Create window
    game->window = SDL_CreateWindow("Outta Here - Mario Game",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SCREEN_WIDTH, SCREEN_HEIGHT,
                                   SDL_WINDOW_SHOWN);
    
    if (game->window == NULL) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    
    // Create renderer
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (game->renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    
    // Initialize game state
    game->running = true;
    memset(game->keys, 0, sizeof(game->keys));
    
    // Initialize player
    if (player_init(&player, game->renderer) != 0) {
        fprintf(stderr, "Failed to initialize player\n");
        return -1;
    }
    
    // Initialize tilemap (with fallback if file doesn't exist)
    tilemap_init(&tilemap, "assets/maps/level1.tmx", game->renderer);
    
    printf("Game initialized successfully\n");
    return 0;
}

void game_cleanup(Game *game) {
    player_cleanup(&player);
    tilemap_cleanup(&tilemap);
    
    if (game->renderer) {
        SDL_DestroyRenderer(game->renderer);
    }
    if (game->window) {
        SDL_DestroyWindow(game->window);
    }
    
    IMG_Quit();
    SDL_Quit();
}

void game_handle_events(Game *game) {
    SDL_Event e;
    
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                game->running = false;
                break;
            case SDL_KEYDOWN:
                game->keys[e.key.keysym.scancode] = true;
                break;
            case SDL_KEYUP:
                game->keys[e.key.keysym.scancode] = false;
                break;
        }
    }
    
    // Handle continuous key presses
    player_handle_input(&player, game);
}

void game_update(Game *game, float dt) {
    player_update(&player, game, dt);
}

void game_render(Game *game) {
    // Clear screen with sky blue color
    SDL_SetRenderDrawColor(game->renderer, 135, 206, 235, 255);
    SDL_RenderClear(game->renderer);
    
    // Render tilemap
    tilemap_render(&tilemap, game->renderer);
    
    // Render player
    player_render(&player, game->renderer);
    
    // Present the rendered frame
    SDL_RenderPresent(game->renderer);
}

void game_run(Game *game) {
    Uint32 last_time = SDL_GetTicks();
    
    printf("Starting game loop\n");
    
    while (game->running) {
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        
        // Cap delta time to prevent large jumps
        if (dt > 0.016f) dt = 0.016f;
        
        game_handle_events(game);
        game_update(game, dt);
        game_render(game);
        
        // Cap framerate to ~60 FPS
        SDL_Delay(16);
    }
}