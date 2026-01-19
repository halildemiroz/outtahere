#include "SDL_events.h"
#include "SDL_render.h"
#include "game.h"
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <menu.h>
#include <stdio.h>

void menuHandleInput(Game* game){
	
	int x,y;

	SDL_Event e;
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_MOUSEBUTTONDOWN)
			SDL_GetMouseState(&x, &y);
	}

	if((x > MENU_START_X - MENU_WIDTH && x < MENU_START_X + MENU_WIDTH) && (y > MENU_START_Y - MENU_HEIGHT && y < MENU_START_Y + MENU_HEIGHT)) game->state = GAME;

}

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

    SDL_Rect startRect = {(SCREEN_WIDTH / 2) - 100,(SCREEN_HEIGHT / 2) - 100,200,200};

	SDL_RenderCopy(game->renderer, game->font->startTexture, NULL, &startRect);
}

void gameScreen(Game* game, Tilemap tm, Player player){

// Clear screen with a background color
	SDL_SetRenderDrawColor(game->renderer, 0, 155, 90, 255);
	SDL_RenderClear(game->renderer);

	// RENDER HERE
		tilemapRender(&tm, game->renderer, &game->cam);
		playerRender(&player, game->renderer, &game->cam);
		/* debug overlay drawing: show object polygons/rects and MTV */
		/* Show the actual collision box (smaller, centered) */
		int collisionOffsetX = (player.width - player.collisionWidth) / 2;
		int collisionOffsetY = (player.height - player.collisionHeight) / 2;
		SDL_Rect pr = {
			(int)player.x + collisionOffsetX, 
			(int)player.y + collisionOffsetY, 
			player.collisionWidth, 
			player.collisionHeight
		};
		if(game->showDebug){
			tilemapDebugRender(&tm, game->renderer, &pr, &game->cam);
		}

		/* Small on-screen indicators for toggle states (top-left) */
		{
			SDL_Rect r1 = {8, 8, 12, 12}; /* smoothing */
			SDL_Rect r2 = {8 + 16, 8, 12, 12}; /* debug */
			SDL_Rect r3 = {8 + 32, 8, 12, 12}; /* culling */
			/* smoothing */
			if(game->cam.smoothingEnabled) SDL_SetRenderDrawColor(game->renderer, 0, 200, 0, 255);
			else SDL_SetRenderDrawColor(game->renderer, 80, 80, 80, 255);
			SDL_RenderFillRect(game->renderer, &r1);
			/* debug */
			if(game->showDebug) 
				SDL_SetRenderDrawColor(game->renderer, 0, 200, 200, 255);
			else
				SDL_SetRenderDrawColor(game->renderer, 80, 80, 80, 255);
			SDL_RenderFillRect(game->renderer, &r2);
			/* culling */
			if(game->cam.optimizeRender) 
				SDL_SetRenderDrawColor(game->renderer, 200, 200, 0, 255);
			else
				SDL_SetRenderDrawColor(game->renderer, 80, 80, 80, 255);
			SDL_RenderFillRect(game->renderer, &r3);
			/* draw borders */
			SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
			SDL_RenderDrawRect(game->renderer, &r1);
			SDL_RenderDrawRect(game->renderer, &r2);
			SDL_RenderDrawRect(game->renderer, &r3);
		}
		//-----------

}

void endScreen(Game *game){
	    SDL_SetRenderDrawColor(game->renderer, 20, 20, 20, 255);
    SDL_RenderClear(game->renderer);

    SDL_Rect endRect = {(SCREEN_HEIGHT / 2) - 100,(SCREEN_WIDTH / 2) - 100,200,200};
    SDL_RenderCopy(game->renderer, game->font->endTexture, NULL, &endRect);

    SDL_RenderPresent(game->renderer);	
}
