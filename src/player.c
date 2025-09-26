#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "game.h"
#include "tilemap.h"
#include <alloca.h>
#include <player.h>

int playerInit(Player *player, SDL_Renderer *renderer, const char* texturePath){
	player->x = SCREEN_WIDTH / 2;
	player->y = SCREEN_HEIGHT / 2;
	player->vx = 0.0f;
	player->vy = 0.0f;
	player->width = PLAYER_WIDTH;
	player->height = PLAYER_HEIGHT;
	player->isOnGround = false;
	player->texture = NULL;

	printf("Player initialized at position %.2f, %.2f\n", player->x, player->y);
	return 0;
}

void playerHandleInput(Player *player, Game *game){
	player->vx = 0.0f;

	if(game->keys[SDL_SCANCODE_A])
		player->vx = -PLAYER_SPEED;
	if(game->keys[SDL_SCANCODE_D])
		player->vx = PLAYER_SPEED;
	/* only allow jumping when on the ground */
	if(game->keys[SDL_SCANCODE_SPACE] && player->isOnGround){
		player->vy = JUMP_SPEED;
		player->isOnGround = false;
	}
}

void playerRender(Player *player, SDL_Renderer *renderer){
	SDL_Rect dest = {(int)player->x, (int)player->y, player->width, player->height};
	if(player->texture)
		SDL_RenderCopy(renderer, player->texture, NULL, &dest);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &dest);
}

void playerUpdate(Player *player, Game *game, float dt){
	/* Axis-separated collision: handle horizontal then vertical */
	if(game->tilemap){
		/* Horizontal move */
		float newX = player->x + player->vx * dt;
		SDL_Rect hr = {(int)newX, (int)player->y, player->width, player->height};
		if(!tilemapCheckCollision(game->tilemap, &hr)){
			player->x = newX;
		}

		/* Apply gravity */
		player->vy += GRAVITY * dt;

		/* Vertical move */
		float newY = player->y + player->vy * dt;
		SDL_Rect vr = {(int)player->x, (int)newY, player->width, player->height};
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
					int overlapLeft = (vr.x + vr.w) - hitObj->rect.x;
					int overlapRight = (hitObj->rect.x + hitObj->rect.w) - vr.x;
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

	/* World bounds */
	if(player->x < 0) player->x = 0;
	if(player->x + player->width > SCREEN_WIDTH) player->x = SCREEN_WIDTH - player->width;
	if(player->y > SCREEN_HEIGHT) player->y = SCREEN_HEIGHT - player->height;
}

void playerClean(Player *player){
	if(player->texture){
		SDL_DestroyTexture(player->texture);
		player->texture = NULL;
	}
}

