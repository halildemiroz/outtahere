#include "SDL_scancode.h"
#include "anim.h"
#include <player.h>
#include <game.h>
#include <tilemap.h>
#include <stdio.h>
#include <stdlib.h>

int playerInit(Player *player, SDL_Renderer *renderer, const char* texturePath){
	if(!player) return -1;
	player->x = 0;
	player->y = 640/*- 32 - PLAYER_HEIGHT*/;
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
	/* try two likely relative paths so assets are found from different working directories */
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

	/* determine movement input and desired state */
	bool left = game->keys[SDL_SCANCODE_A];
	bool right = game->keys[SDL_SCANCODE_D];
	bool runKey = game->keys[SDL_SCANCODE_LSHIFT];
	bool jumpKey = game->keys[SDL_SCANCODE_SPACE];
	
	/* default to standing or maintain jumping state */
	player->vx = 0.0f;
	State desired;
	if (player->isOnGround) {
		desired = IDLE;
	}
	
	if(left || right){
		/* set movement speed but only change animation if on ground */
		float speed;
		if(player->isOnGround) {
			if(runKey) desired = RUN;
			else desired = WALK; /* walking uses the walk animation */
			speed = (desired == RUN) ? RUN_SPEED : WALK_SPEED;
		} else {
			/* Keep jump animation while moving horizontally in air */
			/* DO NOT change the desired state - keep it as JUMP */
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

	/* jump */
	if(jumpKey && player->isOnGround){
		/* Apply jump velocity immediately but set jump state */
		player->vy = JUMP_SPEED;
		player->isOnGround = false;
	}


	/* swap animations only when state actually changes */
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
			default:
				break;
		}
	}

	player->state = desired;
}

void playerRender(Player *player, SDL_Renderer *renderer, Camera* cam){
	int drawX = (int)player->x - (cam ? cam->x : 0);
	int drawY = (int)player->y - (cam ? cam->y : 0);
	
	/* prefer animator rendering when available */
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
	/* Update animator timing */
	animatorUpdate(&player->animator, dt);

	/* Axis-separated collision: handle horizontal then vertical */
	if(game->tilemap){
		/* Calculate collision box offset to center it within the sprite */
		int collisionOffsetX = (player->width - player->collisionWidth) / 2;
		int collisionOffsetY = (player->height - player->collisionHeight) / 2;
		
		/* Horizontal move */
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

		/* Apply gravity */
		player->vy += GRAVITY * dt;

		/* Vertical move */
		float newY = player->y + player->vy * dt;
		SDL_Rect vr = {
			(int)player->x + collisionOffsetX, 
			(int)newY + collisionOffsetY, 
			player->collisionWidth, 
			player->collisionHeight
		};
		/* Check for collision and also fetch the object we hit (if any) */
		TileObject* hitObj = (TileObject*)NULL;
		if(!tilemapCheckCollision(game->tilemap, &vr)){
			player->y = newY;
			player->isOnGround = false;
		} else {
			/* If we hit an object (rect or polygon), resolve against it; otherwise fall back to tile snapping */
			hitObj = tilemapGetCollisionObject(game->tilemap, &vr);
			if(hitObj){
				if(hitObj->hasPolygon){
					/* Use SAT-based MTV to resolve against polygon geometry */
					float nx=0.0f, ny=0.0f, ov=0.0f;
					if(tilemapPolygonRectMTV(hitObj, &vr, &nx, &ny, &ov)){
						/* push player out along MTV */
						player->x += nx * ov;
						player->y += ny * ov;
						/* determine if we landed on top: normal points from polygon -> rect (player),
					   so an upward normal means ny < 0 (screen y downwards) */
						if(ny < -0.5f){
							/* To avoid 'standing on air' when the MTV is tiny or polygon slope is tricky,
						   compute polygon top at player's horizontal span and snap only if close */
							float objTop = FLT_MAX;
							for(int pi=0; pi < hitObj->polygon.pointCount; pi++){
								float py = hitObj->rect.y + hitObj->polygon.points[pi*2 + 1];
								if(py < objTop) objTop = py;
							}
							float playerBottom = player->y + player->height;
							float dist = playerBottom - objTop;
							if(fabsf(dist) <= 3.0f){
								/* snap exactly to polygon top */
								player->y = objTop - player->height;
								player->vy = 0.0f;
								player->isOnGround = true;
							} else {
								/* MTV pushed player but not close enough to count as on-ground */
								player->isOnGround = false;
							}
						} else if(ny > 0.5f){
							/* hit from below */
							player->vy = 0.0f;
							player->isOnGround = false;
						} else {
							/* primarily horizontal normal: cancel horizontal velocity to prevent sticking */
							player->vx = 0.0f;
							/* leave vy as-is or zero small component */
						}
					} else {
						/* fallback: if SAT didn't report intersection (shouldn't happen), treat as rect */
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
					/* simple AABB resolution when hitting an object rect: push player out along minimal axis */
					int overlapLeft = (vr.x + vr.w - 10) - hitObj->rect.x;
					int overlapRight = (hitObj->rect.x + hitObj->rect.w - 10) - vr.x;
					int overlapTop = (vr.y + vr.h) - hitObj->rect.y;
					int overlapBottom = (hitObj->rect.y + hitObj->rect.h) - vr.y;
					/* choose smallest overlap */
					int minOverlap = overlapLeft;
					if(overlapRight < minOverlap) minOverlap = overlapRight;
					if(overlapTop < minOverlap) minOverlap = overlapTop;
					if(overlapBottom < minOverlap) minOverlap = overlapBottom;

					if(minOverlap == overlapTop){
						/* landed on top of object */
						player->y = hitObj->rect.y - player->height;
						player->vy = 0.0f;
						player->isOnGround = true;
					} else if(minOverlap == overlapBottom){
						/* hit the bottom of object */
						player->y = hitObj->rect.y + hitObj->rect.h;
						player->vy = 0.0f;
						player->isOnGround = false;
					} else {
						/* horizontal penetration (rare for vertical move), do not move vertically */
						player->vy = 0.0f;
					}
				}
			} else {
				/* fallback to tile snapping (existing behavior) */
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
		/* Fallback: no tilemap, use simple gravity + ground */
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

	/* World bounds: use tilemap pixel dimensions when available */
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
