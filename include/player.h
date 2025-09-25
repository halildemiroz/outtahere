#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include "game.h"

#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 32
#define PLAYER_SPEED 150.0f
#define JUMP_SPEED -400.0f

typedef struct {
    float x, y;
    float vx, vy;
    int width, height;
    bool on_ground;
    SDL_Texture *texture;
} Player;

// Player functions
int player_init(Player *player, SDL_Renderer *renderer);
void player_cleanup(Player *player);
void player_update(Player *player, Game *game, float dt);
void player_render(Player *player, SDL_Renderer *renderer);
void player_handle_input(Player *player, Game *game);

#endif // PLAYER_H