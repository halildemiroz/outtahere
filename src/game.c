#include "../include/game.h"
#include "../include/tile.h"
#include "../include/map.h"
#include "../include/player.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_keycode.h"
#include "SDL2/SDL_render.h"

#define FPS 60
#define FRAME_DELAY (1000/FPS)

Game game;
Player player;

void gameRun(const char* title, int w, int h){
    gameInit(title,w,h);

    unsigned int frameStart, frameTime;
    
    while(game.isRunning){
        frameStart = SDL_GetTicks();
        
        gameUpdate();
        gameRender();
        gameHandleEvent();

        frameTime = SDL_GetTicks() - frameStart;
        if(frameTime < FRAME_DELAY){
            SDL_Delay(FRAME_DELAY - frameTime);
        }
        
    }
}

void gameInit(const char *title, int width, int height) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    game.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    
    tileInit();

    mapLoad("../assets/outtahere.tmx");
    playerInit();

    game.isRunning = true;

}

void gameRender(){
    SDL_RenderClear(game.renderer);

    mapRender();
    playerRender();
    
    SDL_RenderPresent(game.renderer);
}

void gameUpdate(){
    playerMove();
}

void gameHandleEvent(){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_QUIT:
                game.isRunning = false;
                break;
        }
    }
}

void gameFree(){
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();
}

