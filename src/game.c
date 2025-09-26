#include "game.h"
#include "player.h"
#include <tilemap.h>

static Tilemap tm;
static Player player;

int gameInit(Game* game, const char* title){
	// Check if SDL initialized
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)){
		printf("SDL could not initialized: %s\n", SDL_GetError());
		return -1;
	}
	
	int imgFlag = IMG_INIT_PNG;
	if(!(IMG_Init(imgFlag) & imgFlag)){
		fprintf(stderr, "SDL_image could not initialized! Error: %s\n", IMG_GetError());
		SDL_Quit();
		return -1;
	}

	// Check if window created
	game->window = SDL_CreateWindow(title, 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(!game->window){
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}
	
	// Try to raise/focus the window
	SDL_RaiseWindow(game->window);
	SDL_SetWindowTitle(game->window, title);
	
	// Check if renderer created
	game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!game->renderer){
		// Try software renderer as fallback
		game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_SOFTWARE);

		if(!game->renderer){
			printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
			return -1;
		}
	}

	memset(game->keys, 0, sizeof(game->keys));

	// OTHER INITIALIZATIONS HERE
	tilemapInit(&tm, "../assets/outtahere.tmx", game->renderer);
	/* expose the tilemap to the game so other systems can query collisions */
	game->tilemap = &tm;
	playerInit(&player, game->renderer, "");
	// --------------------------
	
	game->running = true;
	printf("Game initialized\n");
	return 0;
}

void gameHandleEvent(Game* game){
	SDL_Event event;
	// Process all pending SDL events and update keyboard state
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			game->running = false;
		}
		if(event.type == SDL_KEYDOWN){
			if(event.key.keysym.scancode < SDL_NUM_SCANCODES)
				game->keys[event.key.keysym.scancode] = true;
		}
		if(event.type == SDL_KEYUP){
			if(event.key.keysym.scancode < SDL_NUM_SCANCODES)
				game->keys[event.key.keysym.scancode] = false;
		}
	}

	playerHandleInput(&player, game);
	if(game->keys[SDL_SCANCODE_ESCAPE])
		game->running = false;
}

void gameUpdate(Game* game, float dt){
	playerUpdate(&player, game, dt);
}

void gameRender(Game *game){
	// Clear screen with a background color
	SDL_SetRenderDrawColor(game->renderer, 50, 50, 50, 255); // Dark gray background
	SDL_RenderClear(game->renderer);

	// RENDER HERE
		tilemapRender(&tm, game->renderer);
		playerRender(&player, game->renderer);
		/* debug overlay drawing: show object polygons/rects and MTV */
		SDL_Rect pr = {(int)player.x, (int)player.y, player.width, player.height};
		tilemapDebugRender(&tm, game->renderer, &pr);
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
		gameUpdate(game, deltaTime);
		gameRender(game);
		// ---------------------
	
	SDL_Delay(16);
	}
}

void gameClean(Game *game){
	tilemapClean(&tm);
	playerClean(&player);
	SDL_DestroyWindow(game->window);
	SDL_DestroyRenderer(game->renderer);
	IMG_Quit();
	SDL_Quit();
}
