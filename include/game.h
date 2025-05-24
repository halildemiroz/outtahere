#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
} Game;

extern Game game;

void gameRun(const char* title, int w, int h);

void gameInit(const char* title, int width, int height);
void gameRender();
void gameUpdate();
void gameHandleEvent();
void gameFree();

