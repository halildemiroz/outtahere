#include "SDL_scancode.h"
#include "anim.h"
#include <player.h>
#include <game.h>
#include <tilemap.h>
#include <stdio.h>
#include <stdlib.h>

void playerHold(Player* player, Game* game){
	
}

int playerInit(Player *player, SDL_Renderer *renderer, const char* texturePath){
	if(!player) return -1;
	player->x = 0;
	player->y = 640;
	player->vx = 0.0f;
	player->vy = 0.0f;
	player->width = PLAYER_WIDTH;
	player->height = PLAYER_HEIGHT;
	player->collisionWidth = PLAYER_COLLISION_WIDTH;
	player->collisionHeight = PLAYER_COLLISION_HEIGHT;
	player->isOnGround = true;
	player->texture = NULL;
	player->direction = EAST;
	player->state = RUN;
	player->lastState = (State)-1;

	animatorInit(&player->animator);
	char path[512];
	int loaded = 0;
	switch (player->state) {
		case IDLE:
			if(animatorLoadSpritesheet(&player->animator, renderer, "../assets/players/main/idle.png", PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2, 4, 0.12f, true) == 0)
				loaded = 1;
			break;
		case WALK:
			if(animatorLoadSpritesheet(&player->animator, renderer, "../assets/players/main/walk.png", PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2, 6, 0.12f, true) == 0)
				loaded = 1;
			break;
		case RUN:
			if(animatorLoadSpritesheet(&player->animator, renderer, "../assets/players/main/run.png", PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2, 6, 0.12f, true) == 0)
				loaded = 1;
			break;
		case HOLD:
			/* Reuse IDLE for now since we don't have a hold sprite yet */
			if(animatorLoadSpritesheet(&player->animator, renderer, "../assets/players/main/idle.png", PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2, 4, 0.12f, true) == 0)
				loaded = 1;
			break;
		 default:
			break;
	}
	if(!loaded)
		printf("Could not load spritesheet\n");
	else
		printf("Spritesheet successfully loaded\n");

	printf("Player initialized at position %.2f, %.2f\n", player->x, player->y);
	return 0;
}

void playerHandleInput(Player *player, Game *game){
	int tempDustX, tempDustY;

	bool left = game->keys[SDL_SCANCODE_A];
	bool right = game->keys[SDL_SCANCODE_D];
	bool runKey = game->keys[SDL_SCANCODE_LSHIFT];
	bool jumpKey = game->keys[SDL_SCANCODE_SPACE];
	
	player->vx = 0.0f;
	State desired = player->state;
	if (player->isOnGround) {
		desired = IDLE;
	}
	
	if(left || right){
		float speed;
		if(player->isOnGround) {
			if(runKey) desired = RUN;
			else desired = WALK;
			speed = (desired == RUN) ? RUN_SPEED : WALK_SPEED;
		} else {
			speed = WALK_SPEED;
		}
		
		if(left){
			player->vx = -speed;
			player->direction = WEST;
		} else if(right){
			player->vx = speed;
			player->direction = EAST;
		}
	}

	if(jumpKey && player->isOnGround){
		player->vy = JUMP_SPEED;
		player->isOnGround = false;
	}


	if(desired != player->state || player->state == IDLE){
		switch(desired){
			case IDLE:
				animatorLoadSpritesheet(&player->animator, game->renderer, "../assets/players/main/idle.png", PLAYER_WIDTH / 1.5f, PLAYER_HEIGHT / 1.5f, 4, 0.12f, true);
				break;
			case WALK:
				animatorLoadSpritesheet(&player->animator, game->renderer, "../assets/players/main/walk.png", PLAYER_WIDTH / 1.5f, PLAYER_HEIGHT / 1.5f, 6, 0.12f, true);
				break;
			case RUN:
				animatorLoadSpritesheet(&player->animator, game->renderer, "../assets/players/main/run.png", PLAYER_WIDTH / 1.5f, PLAYER_HEIGHT / 1.5f, 6, 0.12f, true);
				break;
			case HOLD:
				animatorLoadSpritesheet(&player->animator, game->renderer, "../assets/players/main/idle.png", PLAYER_WIDTH / 1.5f, PLAYER_HEIGHT / 1.5f, 4, 0.12f, true);
				break;
			default:
				break;
		}
	}

	player->state = desired;
}

void playerRender(Player *player, SDL_Renderer *renderer, Camera* cam){
	int drawX = (int)player->x - (cam ? cam->x : 0);
	int drawY = (int)player->y - (cam ? cam->y : 0);
	
	if(player->animator.texture){
		SDL_RendererFlip flip = (player->direction == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	
		animatorRender(&player->animator, renderer, drawX, drawY, player->width, player->height, flip);
		return;
	}

	SDL_Rect dest = {drawX, drawY, player->width, player->height};
	if(player->texture)
		SDL_RenderCopy(renderer, player->texture, NULL, &dest);
	else {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &dest);
	}
}

void playerUpdate(Player *player, Game *game, float dt){
	animatorUpdate(&player->animator, dt);

	// HOLD MECHANIC
	// if(game->tilemap && !player->isOnGround){
	// 	int collisionOffsetX = (player->width - player->collisionWidth) / 2;
	// 	int collisionOffsetY = (player->height - player->collisionHeight) / 2;
	//
	// 	SDL_Rect checkLeft = {
	// 		(int)player->x + collisionOffsetX - 1,
	// 		(int)player->y + collisionOffsetY,
	// 		1,
	// 		player->collisionHeight
	// 	};
	// 	SDL_Rect checkRight = {
	// 		(int)player->x + collisionOffsetX + player->collisionWidth,
	// 		(int)player->y + collisionOffsetY,
	// 		1,
	// 		player->collisionHeight
	// 	};
	//
	// 	if(tilemapCheckCollision(game->tilemap, &checkLeft) || 
	// 	   tilemapCheckCollision(game->tilemap, &checkRight)){
	// 		player->state = HOLD;
	// 		player->vy = 0.0f;
	// 	}
	// }

	if(game->tilemap){
		int collisionOffsetX = (player->width - player->collisionWidth) / 2;
		int collisionOffsetY = (player->height - player->collisionHeight) / 2;
		
		float newX = player->x + player->vx * dt;
		SDL_Rect hr = {
			(int)newX + collisionOffsetX, 
			(int)player->y + collisionOffsetY, 
			player->collisionWidth, 
			player->collisionHeight
		};
		if(!tilemapCheckCollision(game->tilemap, &hr)){
			player->x = newX;
		}

		if(player->state != HOLD)
			player->vy += GRAVITY * dt;

		float newY = player->y + player->vy * dt;
		SDL_Rect vr = {
			(int)player->x + collisionOffsetX, 
			(int)newY + collisionOffsetY, 
			player->collisionWidth, 
			player->collisionHeight
		};
		TileObject* hitObj = (TileObject*)NULL;
		if(!tilemapCheckCollision(game->tilemap, &vr)){
			player->y = newY;
			player->isOnGround = false;
		} else {
			hitObj = tilemapGetCollisionObject(game->tilemap, &vr);
			if(hitObj){
				if(hitObj->hasPolygon){
					float nx=0.0f, ny=0.0f, ov=0.0f;
					if(tilemapPolygonRectMTV(hitObj, &vr, &nx, &ny, &ov)){
						player->x += nx * ov;
						player->y += ny * ov;

						if(ny < -0.5f){

							float objTop = FLT_MAX;
							for(int pi=0; pi < hitObj->polygon.pointCount; pi++){
								float py = hitObj->rect.y + hitObj->polygon.points[pi*2 + 1];
								if(py < objTop) objTop = py;
							}
							float playerBottom = player->y + player->height;
							float dist = playerBottom - objTop;
							if(fabsf(dist) <= 3.0f){
								player->y = objTop - player->height;
								player->vy = 0.0f;
								player->isOnGround = true;
							} else {
								player->isOnGround = false;
							}
						} else if(ny > 0.5f){
							player->vy = 0.0f;
							player->isOnGround = false;
						} else {
							player->vx = 0.0f;
						}
					} else {
						int overlapLeft = (vr.x + vr.w) - hitObj->rect.x;
						int overlapRight = (hitObj->rect.x + hitObj->rect.w) - vr.x;
						int overlapTop = (vr.y + vr.h) - hitObj->rect.y;
						int overlapBottom = (hitObj->rect.y + hitObj->rect.h) - vr.y;
						int minOverlap = overlapLeft;
						if(overlapRight < minOverlap) minOverlap = overlapRight;
						if(overlapTop < minOverlap) minOverlap = overlapTop;
						if(overlapBottom < minOverlap) minOverlap = overlapBottom;
						if(minOverlap == overlapTop){
							player->y = hitObj->rect.y - player->height;
							player->vy = 0.0f;
							player->isOnGround = true;
						} else if(minOverlap == overlapBottom){
							player->y = hitObj->rect.y + hitObj->rect.h;
							player->vy = 0.0f;
							player->isOnGround = false;
						} else {
							player->vy = 0.0f;
						}
					}
				} else {
					int overlapLeft = (vr.x + vr.w - 10) - hitObj->rect.x;
					int overlapRight = (hitObj->rect.x + hitObj->rect.w - 10) - vr.x;
					int overlapTop = (vr.y + vr.h) - hitObj->rect.y;
					int overlapBottom = (hitObj->rect.y + hitObj->rect.h) - vr.y;
					int minOverlap = overlapLeft;
					if(overlapRight < minOverlap) minOverlap = overlapRight;
					if(overlapTop < minOverlap) minOverlap = overlapTop;
					if(overlapBottom < minOverlap) minOverlap = overlapBottom;

					if(minOverlap == overlapTop){
						player->y = hitObj->rect.y - player->height;
						player->vy = 0.0f;
						player->isOnGround = true;
					} else if(minOverlap == overlapBottom){
						player->y = hitObj->rect.y + hitObj->rect.h;
						player->vy = 0.0f;
						player->isOnGround = false;
					} else {
						player->vy = 0.0f;
					}
				}
			} else {
				int tileH = game->tilemap->map->tile_height;
				int topTile = (int)floor(newY / tileH);
				int bottomTile = (int)floor((newY + player->height - 1) / tileH);

				if(player->vy > 0){
					player->y = bottomTile * tileH - player->height;
					player->vy = 0.0f;
					player->isOnGround = true;
				} else if(player->vy < 0){
					player->y = (topTile + 1) * tileH;
					player->vy = 0.0f;
					player->isOnGround = false;
				}
			}
		}
	} else {
		player->x += player->vx * dt;
		player->vy += GRAVITY * dt;
		player->y += player->vy * dt;
		float groundLevel = SCREEN_HEIGHT - 32;
		if(player->y + player->height >= groundLevel){
			player->y = groundLevel - player->height;
			player->vy = 0.0f;
			player->isOnGround = true;
		}
	}

	int worldW = SCREEN_WIDTH;
	int worldH = SCREEN_HEIGHT;
	if(game && game->tilemap && game->tilemap->map){
		worldW = game->tilemap->map->width * game->tilemap->map->tile_width;
		worldH = game->tilemap->map->height * game->tilemap->map->tile_height;
	}
	if(player->x < 0) player->x = 0;
	if(player->x + player->width > worldW) player->x = worldW - player->width;
	if(player->y + player->height > worldH) player->y = worldH - player->height;
}

void playerClean(Player *player){
	animatorFree(&player->animator);
	if(player->texture){
		SDL_DestroyTexture(player->texture);
		player->texture = NULL;
	}
}
