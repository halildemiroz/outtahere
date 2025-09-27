#include <anim.h>
#include <stdio.h>
#include <stdlib.h>

void animatorInit(Animator* a){
	if(!a) return;
	a->texture = NULL;
	a->surface = NULL;
	a->frames = NULL;
	a->frameCount = 0;
	a->frameTime = 0.0f;
	a->elapsed = 0.0f;
	a->currentFrame = 0;
	a->loop = true;
	a->isPlaying = false;
}

int animatorLoadSingle(Animator* a, SDL_Renderer* r, const char* path){
	if(!a || !r || !path) return -1;
	animatorFree(a);
	SDL_Surface* surf = IMG_Load(path);
	if(!surf){
		fprintf(stderr, "Failed to load image '%s': %s\n", path, IMG_GetError());
		return -1;
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
	if(!tex){
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return -1;
	}
	a->texture = tex;
	a->surface = surf; /* keep surface for possible future use */
	a->frameCount = 1;
	a->frames = NULL;
	a->frameTime = 0.1f;
	a->elapsed = 0.0f;
	a->currentFrame = 0;
	a->loop = true;
	a->isPlaying = true;
	/* Log successful single-image load for debugging */
	if(surf){
		fprintf(stdout, "Loaded image '%s' as single-frame (%dx%d)\n", path, surf->w, surf->h);
	}
	return 0;
}

int animatorLoadSpritesheet(Animator* a, SDL_Renderer* r, const char* path, int tileW, int tileH, int frameCount, float frameTime, bool loop){
	if(!a || !r || !path || tileW <= 0 || tileH <= 0) return -1;
	animatorFree(a);
	SDL_Surface* surf = IMG_Load(path);
	if(!surf){
		fprintf(stderr, "Failed to load spritesheet '%s': %s\n", path, IMG_GetError());
		return -1;
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
	if(!tex){
		fprintf(stderr, "Failed to create texture from spritesheet: %s\n", SDL_GetError());
		SDL_FreeSurface(surf);
		return -1;
	}
	a->texture = tex;
	a->surface = surf;
	int cols = surf->w / tileW;
	int rows = surf->h / tileH;
	int maxFrames = cols * rows;
	if(frameCount <= 0 || frameCount > maxFrames) frameCount = maxFrames;
	a->frameCount = frameCount;
	a->frames = (SDL_Rect*)malloc(sizeof(SDL_Rect) * frameCount);
	if(!a->frames){
		fprintf(stderr, "Out of memory allocating frames\n");
		animatorFree(a);
		return -1;
	}
	/* fill frame rects left-to-right, top-to-bottom */
	int fi = 0;
	for(int ry = 0; ry < rows && fi < frameCount; ry++){
		for(int cx = 0; cx < cols && fi < frameCount; cx++){
			a->frames[fi].x = cx * tileW;
			a->frames[fi].y = ry * tileH;
			a->frames[fi].w = tileW;
			a->frames[fi].h = tileH;
			fi++;
		}
	}
	a->frameTime = frameTime > 0.0f ? frameTime : 0.12f;
	a->elapsed = 0.0f;
	a->currentFrame = 0;
	a->loop = loop;
	a->isPlaying = (a->frameCount > 1);
	fprintf(stdout, "Loaded spritesheet '%s' (%dx%d) tiles %dx%d -> frames=%d cols=%d rows=%d frameTime=%.3f\n", path, surf->w, surf->h, tileW, tileH, a->frameCount, cols, rows, a->frameTime);
	return 0;
}

void animatorFree(Animator* a){
	if(!a) return;
	if(a->texture){
		SDL_DestroyTexture(a->texture);
		a->texture = NULL;
	}
	if(a->surface){
		SDL_FreeSurface(a->surface);
		a->surface = NULL;
	}
	if(a->frames){
		free(a->frames);
		a->frames = NULL;
	}
	a->frameCount = 0;
	a->elapsed = 0.0f;
	a->currentFrame = 0;
	a->isPlaying = false;
}

void animatorUpdate(Animator* a, float dt){
	if(!a || !a->isPlaying || a->frameCount <= 1) return;
	a->elapsed += dt;
	while(a->elapsed >= a->frameTime){
		a->elapsed -= a->frameTime;
		a->currentFrame++;
		if(a->currentFrame >= a->frameCount){
			if(a->loop) a->currentFrame = 0;
			else { a->currentFrame = a->frameCount - 1; a->isPlaying = false; }
		}
	}
}

void animatorRender(Animator* a, SDL_Renderer* r, int x, int y, int w, int h, SDL_RendererFlip flip){
	if(!a || !r) return;
	if(a->texture){
		SDL_Rect dst = {x, y, w, h};
		if(a->frames && a->frameCount > 0){
			SDL_Rect src = a->frames[a->currentFrame];
			SDL_RenderCopyEx(r, a->texture, &src, &dst, 0.0, NULL, flip);
		} else {
			SDL_RenderCopyEx(r, a->texture, NULL, &dst, 0.0, NULL, flip);
		}
	}
}
