#include "../include/player.h"
#include "tmx.h"

Uint32 lastFrameTime = 0;
const int FRAME_DELAY = 100;

Player player;

void playerInit(){
    player.src.h = 96;
    player.src.w = 96;
    player.src.x = 0;
    player.src.y = 0;

    player.dest.h = 96;
    player.dest.w = 96;
    player.dest.x = 200;
    player.dest.y = 120;

    player.collision.h = 80;
    player.collision.w = 60;
    player.collision.x = player.dest.x + (player.dest.w - player.collision.w) / 2;
    player.collision.y = player.dest.y + (player.dest.h - player.collision.h);

    player.state = IDLE;
    player.currentFrame = 0;
    player.idleFrames = 10;
    player.runFrames = 16;
    player.vx = 2;
    player.isGrounded = false;

    player.idleSurface = IMG_Load("../assets/IDLE.png");
    player.runSurface = IMG_Load("../assets/RUN.png");
    player.idleTexture = SDL_CreateTextureFromSurface(game.renderer, player.idleSurface);
    player.runTexture = SDL_CreateTextureFromSurface(game.renderer, player.runSurface);
    SDL_FreeSurface(player.idleSurface);
    SDL_FreeSurface(player.runSurface);
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

void playerMove(tmx_map* mapData){
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

    int prevX = player.dest.x;
    int prevY = player.dest.y;
    
    // check if player is collided with border
    // if yes put player away from border
    if(playerCheckWindowBorderCollision()){
       if(player.dest.x < 30)
           player.dest.x = prevX + 30;
       if(player.dest.x > 900)
           player.dest.x = prevX - 100;
       player.dest.y = prevY - 20;
    }

    // Horizontal Movement
    if (keystate[SDL_SCANCODE_D]) {
        player.dest.x += player.vx;
        moved = true;
        player.direction = FRIGHT;
    }
    
    if (keystate[SDL_SCANCODE_D] && keystate[SDL_SCANCODE_LSHIFT]) {
        player.dest.x += player.vx * 2;
        moved = true;
        player.direction = FRIGHT;
    }

    if (keystate[SDL_SCANCODE_A]) {
        player.dest.x -= player.vx;
        moved = true;
        player.direction = FLEFT;
    }

    if(keystate[SDL_SCANCODE_A] && keystate[SDL_SCANCODE_LSHIFT]){
        player.dest.x -= player.vx * 2;
        moved = true;
        player.direction = FLEFT;
    }
    // --------

    // Vertical Movement
    player.vy += G;
    if(player.vy > TERMINAL_VELOCITY)
        player.vy = TERMINAL_VELOCITY;
    player.dest.y += player.vy;

    if(playerCheckBottomCollision(mapData)){
        player.vy = 0;
        player.dest.y = prevY;
        player.isGrounded = true;
    }

    if(keystate[SDL_SCANCODE_SPACE] && player.isGrounded){
        player.vy = JUMP_FORCE;
        player.isGrounded = false;
    }
    // --------
    
    // Reset Player
    if(keystate[SDL_SCANCODE_R]){
        player.dest.x = 200;
        player.dest.y = 120;
    }

    if (moved) {
        player.state = RUN;
    }
    else if (player.state != IDLE) {
        player.state = IDLE;
    }

}

bool playerCheckBottomCollision(tmx_map* mapData){
    // initialize player bottom x and y coords
    int pblx = player.dest.x;
    int pbrx = player.dest.x + player.dest.w; 
    int pby = player.dest.y + (player.dest.w / 2) + 30;

    return (checkPointCollision(mapData, pblx, pby) || checkPointCollision(mapData, pbrx, pby));
}

bool playerCheckWindowBorderCollision(){
    // return true if there is collision
    if(player.dest.x < 0 || player.dest.x > 930 || player.dest.y > 640)
        return true;
    return false;
}

void playerFree(){
    SDL_DestroyTexture(player.idleTexture);
    SDL_DestroyTexture(player.runTexture);
}
