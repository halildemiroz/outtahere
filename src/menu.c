#include "SDL_render.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <menu.h>
#include <stdio.h>

void startScreen(Game *game){
	TTF_Init();
	game->font->font = TTF_OpenFont("../assets/font.ttf", 300);
	if(game->font->font == NULL){
		fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
		return;
	}
	
	SDL_Color white = {255,255,255,255};

	game->font->fontSurface = TTF_RenderText_Solid(game->font->font, "Start", white);

	game->font->fontTexture = SDL_CreateTextureFromSurface(game->renderer, game->font->fontSurface);
	if(game->font->fontTexture == NULL){
		fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
		TTF_CloseFont(game->font->font);
		SDL_FreeSurface(game->font->fontSurface);
		return;
	}

	SDL_Rect startRect = {200,200,200,200};
	SDL_RenderCopy(game->renderer, game->font->fontTexture, NULL, &startRect);
	
	SDL_FreeSurface(game->font->fontSurface);
	SDL_DestroyTexture(game->font->fontTexture);
	TTF_CloseFont(game->font->font);
}

void gameScreen(Game* game){
	return;
}

void endScreen(Game *game){
	return;
}
