#ifndef PLAYER_H
#define PLAYER_H

#include <anim.h>
#include <game.h>

#define PLAYER_WIDTH 48
#define PLAYER_HEIGHT 48
#define PLAYER_COLLISION_WIDTH 28
#define PLAYER_COLLISION_HEIGHT 48
#define WALK_SPEED 150.0f
#define RUN_SPEED 225.0f
#define JUMP_SPEED -400.0f
#define GRAVITY 800.0f

typedef enum{

	NORTH = 0, WEST, SOUTH, EAST
}	Direction;

typedef enum{
	IDLE = 0, WALK, RUN, JUMP
} State;

typedef struct{
	float x, y;
	float vx, vy;
	int width, height;      
	int collisionWidth, collisionHeight;
	bool isOnGround;
	SDL_Texture* texture;
	Animator animator;
	Direction direction;
	State state;
	State lastState;
} Player;

int playerInit(Player* player, SDL_Renderer* renderer, const char* texturePath);

void playerClean(Player* player);
void playerUpdate(Player* player, Game* game, float dt);
void playerRender(Player* player, SDL_Renderer* renderer, Camera* cam);
void playerHandleInput(Player* player, Game* game);

#endif
