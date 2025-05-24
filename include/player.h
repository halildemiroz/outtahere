#pragma once

#include "game.h"

#define G 10.0f

typedef enum{
    IDLE = 0,
    RUN
} State;

typedef struct {
    SDL_Surface* idleSurface;
    SDL_Surface* runSurface;
    SDL_Texture* idleTexture;
    SDL_Texture* runTexture;
    SDL_Rect src, dest;

    int idleFrames, runFrames, currentFrame;
    int health;
    float vx, vy, a;
    State state;
} Player;
extern Player player;

void playerInit();
void playerRender();
void playerMove();

bool playerCanMove();

