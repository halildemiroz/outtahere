#include "SDL_render.h"
#include "game.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <menu.h>

void loadScreenAssets(Game* game){

	SDL_Color white = {255,255,255,255};
	game->font->startSurface = TTF_RenderText_Solid(game->font->font, "Start", white);

	game->font->endSurface = TTF_RenderText_Solid(game->font->font, "End", white);
	
	game->font->startTexture = SDL_CreateTextureFromSurface(game->renderer, game->font->startSurface);
	game->font->endTexture = SDL_CreateTextureFromSurface(game->renderer, game->font->endSurface);

	SDL_FreeSurface(game->font->endSurface);
	SDL_FreeSurface(game->font->startSurface);
}

void startScreen(Game *game){
	SDL_SetRenderDrawColor(game->renderer, 20, 20, 20, 255);
    SDL_RenderClear(game->renderer);

    SDL_Rect startRect = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 200,300};
    SDL_RenderCopy(game->renderer, game->font->startTexture, NULL, &startRect);
}



void endScreen(Game *game){
	SDL_SetRenderDrawColor(game->renderer, 20, 20, 20, 255);
    SDL_RenderClear(game->renderer);

    SDL_Rect endRect = {200,200,200,200};
    SDL_RenderCopy(game->renderer, game->font->endTexture, NULL, &endRect);

    SDL_RenderPresent(game->renderer);	
}
