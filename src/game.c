#include "SDL2/SDL.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_video.h"
#include <game.h>

int gameInit(Game* game, const char* title){
	// Check if SDL initialized
	if(SDL_Init(SDL_INIT_EVERYTHING)){
		printf("SDL could not initialized\n");
		return -1;
	}

	// Check if window created
	game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if(!game->window){
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}
	
	// Check if renderer created
	game->renderer = SDL_CreateRenderer(game->window, -1,SDL_RENDERER_ACCELERATED);
	if(!game->renderer){
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	// OTHER INITIALIZATIONS HERE
	
	// --------------------------
	
	// If everyting is initialized
	game->running = true;
	printf("Game initialized");
	return 0;
}

void gameHandleEvent(Game* game){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch (e.type) {
			case SDL_QUIT:
				game->running = false;
				break;
			case SDL_KEYDOWN:
				if(e.key.keysym.sym == SDLK_q){
					game->running = false;
				}
				break;
			default:
				break;
		}
	}
}

void gameUpdate(Game* game){}

void gameRender(Game *game){
	SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
	SDL_RenderClear(game->renderer);

	// RENDER HERE

	//-----------
	
	SDL_RenderPresent(game->renderer);
}

void gameRun(Game *game){
	Uint32 lastTime = SDL_GetTicks();
	printf("Starting game loop\n");

	while(game->running){
		Uint32 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		if(deltaTime > 0.016f)
			deltaTime = 0.016f;
	
		// OTHER GAME FUNCS HERE
		gameHandleEvent(game);
		gameUpdate(game);
		gameRender(game);
		// ---------------------
	
	SDL_Delay(16);
	}
}

void gameClean(Game *game){
	SDL_DestroyWindow(game->window);
	SDL_DestroyRenderer(game->renderer);
	SDL_Quit();
}
