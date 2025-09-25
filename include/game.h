#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 32

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running;
    bool keys[SDL_NUM_SCANCODES];
} Game;

// Game functions
int game_init(Game *game);
void game_cleanup(Game *game);
void game_handle_events(Game *game);
void game_update(Game *game, float dt);
void game_render(Game *game);
void game_run(Game *game);

#endif // GAME_H