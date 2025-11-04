#ifndef MENU_H
#define MENU_H

#include <game.h>
#include <SDL_ttf.h>

typedef struct{	
	TTF_Font* font;
	SDL_Rect rect;
} Menu;

void menuRenderGame(Menu* menu, const char* fontPath, SDL_Renderer* renderer);

#endif
