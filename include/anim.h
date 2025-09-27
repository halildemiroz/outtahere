#ifndef ANIM_H
#define ANIM_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

typedef struct {
	SDL_Texture* texture;
	SDL_Surface* surface;
	SDL_Rect* frames; /* optional array of source rects */
	int frameCount;
	float frameTime; /* seconds per frame */
	float elapsed;
	int currentFrame;
	bool loop;
	bool isPlaying;
} Animator;

/* Initialize animator to zeroed state */
void animatorInit(Animator* a);

/* Load a single image as a 1-frame animation */
int animatorLoadSingle(Animator* a, SDL_Renderer* r, const char* path);

/* Load a spritesheet: frames are tileW x tileH in the image. If frameCount<=0 it'll auto-calc. */
int animatorLoadSpritesheet(Animator* a, SDL_Renderer* r, const char* path, int tileW, int tileH, int frameCount, float frameTime, bool loop);

/* Free animator resclear
 * clear
 *
 * urces */
void animatorFree(Animator* a);

/* Update animation timing (dt in seconds) */
void animatorUpdate(Animator* a, float dt);

/* Render current frame at (x,y) with size w,h and flip */
void animatorRender(Animator* a, SDL_Renderer* r, int x, int y, int w, int h, SDL_RendererFlip flip);

#endif
