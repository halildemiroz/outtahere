#include "../include/player.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_keyboard.h"
#include "SDL2/SDL_keycode.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_stdinc.h"
#include "SDL2/SDL_timer.h"

Uint32 lastFrameTime = 0;
const int FRAME_DELAY = 100;

Player player;

void playerInit(){
    player.src.h = 96;
    player.src.w = 96;
    player.src.x = 0;
    player.src.y = 0;
    player.dest.h = 144;
    player.dest.w = 144;
    player.dest.x = 200;
    player.dest.y = 120;
    player.state = IDLE;
    player.currentFrame = 0;
    player.idleFrames = 10; // 10 frames for IDLE animation
    player.runFrames = 16;

    player.idleSurface = IMG_Load("../assets/IDLE.png");
    player.runSurface = IMG_Load("../assets/RUN.png");
    player.idleTexture = SDL_CreateTextureFromSurface(game.renderer, player.idleSurface);
    player.runTexture = SDL_CreateTextureFromSurface(game.renderer, player.runSurface);
    SDL_FreeSurface(player.idleSurface);
}

void playerAnimate(int frame){
    Uint32 currentTime = SDL_GetTicks();

    if(currentTime > lastFrameTime + FRAME_DELAY){
        player.currentFrame++;
        if(player.currentFrame >= frame){
            player.currentFrame = 0;
        }
        player.src.x = player.currentFrame * player.src.w;
        lastFrameTime = currentTime;
    }
   
}

void playerRender(){
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if(player.direction == FLEFT)
        flip = SDL_FLIP_HORIZONTAL;

    switch (player.state) {
        case IDLE:
            if (player.idleTexture) {
                playerAnimate(player.idleFrames);
                SDL_RenderCopyEx(game.renderer, player.idleTexture, &player.src, &player.dest, 0, NULL, flip);
            }
            break;
        case RUN:
            if(player.runTexture){
                playerAnimate(player.runFrames);
                SDL_RenderCopyEx(game.renderer, player.runTexture, &player.src, &player.dest, 0, NULL, flip);
            }
            break;
    }
}

void playerMove(){
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a) {
                player.state = IDLE;
            }
        }
    }

    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    bool moved = false;

    if (keystate[SDL_SCANCODE_D]) {
        player.dest.x += SPEED;
        moved = true;
        player.direction = FRIGHT;
    }
    if (keystate[SDL_SCANCODE_D] && keystate[SDL_SCANCODE_LSHIFT]) {
        player.dest.x += SPEED * 2;
        moved = true;
        player.direction = FRIGHT;
    }

    if (keystate[SDL_SCANCODE_A]) {
        player.dest.x -= SPEED;
        moved = true;
        player.direction = FLEFT;
    }
    if(keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_LSHIFT]){
        player.dest.x -= SPEED * 2;
        moved = true;
        player.direction = FLEFT;
    }

    if (moved) {
        player.state = RUN;
    }
    else if (player.state != IDLE) {
        player.state = IDLE;
    }

}


