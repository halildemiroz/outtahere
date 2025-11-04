#include "SDL_ttf.h"
#include <menu.h>

void menuRenderGame(Menu* menu, const char *fontPath, SDL_Renderer *renderer){
	TTF_Init();
	
	menu->font = TTF_OpenFont(fontPath, 300);
	if(!menu->font){
		fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
		return;
	}
	SDL_Color white = {255,255,255,255};
}
