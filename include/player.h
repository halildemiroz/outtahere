#ifndef PLAYER_H
#define PLAYER_H

#include <anim.h>
#include <game.h>

#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 32
#define WALK_SPEED 150.0f
#define RUN_SPEED 225.0f
#define JUMP_SPEED -400.0f
#define GRAVITY 400.0f

typedef enum{

// 0					1				2			3
	NORTH = 0, WEST, SOUTH, EAST
}	Direction;

typedef enum{
	IDLE = 0, WALK, RUN, JUMP
} 	 State;

typedef struct{
	float x, y;
	float vx, vy;
	int width, height;
	bool isOnGround;
	SDL_Texture* texture;
	Animator animator; /* animation controller */
	Direction direction;
	State state;
	State lastState;
} Player;

int playerInit(Player* player, SDL_Renderer* renderer, const char* texturePath);

void playerClean(Player* player);
void playerUpdate(Player* player, Game* game, float dt);
void playerRender(Player* player, SDL_Renderer* renderer);
void playerHandleInput(Player* player, Game* game);

#endif
