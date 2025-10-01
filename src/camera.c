#include "camera.h"
#include <math.h>

static float lerp(float a, float b, float t){
	return a + (b - a) * t;
}

void updateCamera(Camera *cam, SDL_Rect player, int mapWidth, int mapHeight){
	/* Target position centers the camera on the player */
	float targetX = player.x + player.w / 2.0f - cam->w / 2.0f;
	float targetY = player.y + player.h / 2.0f - cam->h / 2.0f;

	/* Clamp target to map bounds */
	if(targetX < 0.0f) targetX = 0.0f;
	if(targetY < 0.0f) targetY = 0.0f;
	if(targetX > (float)(mapWidth - cam->w)) targetX = (float)(mapWidth - cam->w);
	if(targetY > (float)(mapHeight - cam->h)) targetY = (float)(mapHeight - cam->h);

	/* Initialize floating positions if needed */
	if(cam->fx == 0.0f && cam->fy == 0.0f && (cam->x != 0 || cam->y != 0)){
		cam->fx = (float)cam->x;
		cam->fy = (float)cam->y;
	}

	/* Smooth or immediate update */
	if(cam->smoothingEnabled){
		float t = cam->lerpFactor;
		if(t < 0.0001f) t = 0.15f; /* sensible default */
		cam->fx = lerp(cam->fx, targetX, t);
		cam->fy = lerp(cam->fy, targetY, t);
	} else {
		cam->fx = targetX;
		cam->fy = targetY;
	}

	/* Write back integer camera coordinates */
	cam->x = (int)roundf(cam->fx);
	cam->y = (int)roundf(cam->fy);

	/* Final clamp to ensure integers are within bounds */
	if(cam->x < 0) cam->x = 0;
	if(cam->y < 0) cam->y = 0;
	if(cam->x > mapWidth - cam->w) cam->x = mapWidth - cam->w;
	if(cam->y > mapHeight - cam->h) cam->y = mapHeight - cam->h;
}
