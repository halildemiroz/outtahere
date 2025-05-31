#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "map.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    Map map;
} Game;

extern Game game;

void gameRun(const char* title, int w, int h);

void gameInit(const char* title, int width, int height);
void gameRender();
void gameUpdate();
void gameHandleEvent();
void gameFree();

#endif
