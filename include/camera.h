#ifndef CAMERA_H
#define CAMERA_H

#include <anim.h>
#include <stdbool.h>

typedef struct{
	int w, h;
	int x, y;
	float fx, fy; /* floating point positions used for smooth lerp */
	float lerpFactor; /* smoothing factor (0..1) */
	bool smoothingEnabled;
	bool optimizeRender; /* whether to cull tiles outside camera */
} Camera;

void updateCamera(Camera* cam, SDL_Rect player, int mapWidth, int mapHeight);

#endif
