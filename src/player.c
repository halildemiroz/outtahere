#include "../include/player.h"

Uint32 lastFrameTime = 0;
const int FRAME_DELAY = 100;

Player player;

void playerInit(){
    player.src.h = 64;
    player.src.w = 64;
    player.src.x = 0;
    player.src.y = 0;
    player.dest.h = 64;
    player.dest.w = 64;
    player.dest.x = 100;
    player.dest.y = 100;
    player.state = IDLE;
    player.currentFrame = 0;
    player.idleFrames = 10; // 10 frames for IDLE animation
    player.runFrames = 7;

    player.idleSurface = IMG_Load("../assets/IDLE.png");
    // player.runSurface = IMG_Load("../assets/RUN.png");
    player.idleTexture = SDL_CreateTextureFromSurface(game.renderer, player.idleSurface);
    SDL_FreeSurface(player.idleSurface);
}

void playerRender(){
    Uint32 currentTime = SDL_GetTicks();
    
    // Update animation frame at regular intervals
    if (currentTime > lastFrameTime + FRAME_DELAY) {

        switch (player.state){
            case IDLE:
                player.currentFrame = (player.currentFrame + 1) % player.idleFrames;
                break;
            case RUN:
                player.currentFrame = (player.currentFrame + 1) % player.runFrames;
                break;
        }
        lastFrameTime = currentTime;
        
        player.src.x = player.currentFrame * player.src.w;
    }
    
    switch (player.state) {
        case IDLE:
            if (player.idleTexture) {
                SDL_RenderCopy(game.renderer, player.idleTexture, &player.src, &player.dest);
            }
            break;
        case RUN:
            break;
    }
}
