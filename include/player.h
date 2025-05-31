#pragma once

#include "game.h"
#include "tile.h"
#include "tmx.h"

#define G 1
#define JUMP_FORCE -12
#define TERMINAL_VELOCITY 10

typedef enum{
    FRIGHT = 0,
    FLEFT
} Direction;

typedef enum{
    IDLE = 0,
    RUN
} State;

typedef struct {
    SDL_Surface* idleSurface;
    SDL_Surface* runSurface;
    SDL_Texture* idleTexture;
    SDL_Texture* runTexture;
    SDL_Rect src, dest, collision;

    bool isGrounded;

    int idleFrames, runFrames, currentFrame;
    int health;

    float vx, vy, a;

    Direction direction;
    State state;
} Player;
extern Player player;


void playerInit();
void playerRender();
void playerMove(tmx_map* mapData);
void playerAnimate(int frame);
void playerFree();

bool playerCheckBottomCollision(tmx_map* mapData);
bool playerCheckWindowBorderCollision();

