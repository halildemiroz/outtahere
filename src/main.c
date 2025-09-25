#include <stdio.h>
#include <SDL2/SDL.h>
#include "tilemap.h"

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window *window = SDL_CreateWindow("Test", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        800, 600, SDL_WINDOW_SHOWN);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    Tilemap tilemap;
    int result = tilemap_init(&tilemap, "test.tmx", renderer);
    
    printf("Tilemap init result: %d\n", result);
    
    tilemap_cleanup(&tilemap);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}