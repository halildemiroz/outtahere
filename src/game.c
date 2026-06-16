#include "SDL_render.h"
#include "anim.h"
#include "tilemap.h"
#include "player.h"
#include "game.h"
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_ttf.h>
#include <menu.h>

static Tilemap tm;
static Player player;
static Animator a;

static const char *levelPaths[] = {
	"../assets/tilemap/demo.tmx",
	"../assets/tilemap/level2.tmx"
};
static const int levelCount = (int)(sizeof(levelPaths) / sizeof(levelPaths[0]));
static int currentLevel = 0;
static bool lastInteractKey = false;

static bool findSpawnPoint(tmx_map *map, float *spawnX, float *spawnY){
	if(!map || !spawnX || !spawnY) return false;

	const char *spawnLayerNames[] = {"spawn", "spawns", "player_spawn", "player-spawn"};
	for(size_t i = 0; i < sizeof(spawnLayerNames) / sizeof(spawnLayerNames[0]); i++){
		tmx_layer *layer = tmx_find_layer_by_name(map, spawnLayerNames[i]);
		if(layer && layer->type == L_OBJGR && layer->content.objgr && layer->content.objgr->head){
			tmx_object *fallbackObj = layer->content.objgr->head;
			for(tmx_object *obj = layer->content.objgr->head; obj; obj = obj->next){
				if(obj->obj_type == OT_POINT){
					*spawnX = (float)obj->x;
					*spawnY = (float)obj->y;
					return true;
				}
			}
			*spawnX = (float)fallbackObj->x;
			*spawnY = (float)fallbackObj->y;
			return true;
		}
	}

	return false;
}

static void resetPlayerSpawn(Player *p, float spawnX, float spawnY){
	if(!p) return;
	p->x = spawnX;
	p->y = spawnY;
	p->vx = 0.0f;
	p->vy = 0.0f;
	p->isOnGround = true;
	p->direction = EAST;
}

static bool loadLevel(Game *game, int levelIndex){
	if(!game || levelIndex < 0 || levelIndex >= levelCount) return false;

	tilemapClean(&tm);
	if(tilemapInit(&tm, levelPaths[levelIndex], game->renderer) != 0){
		fprintf(stderr, "Failed to load level %d: %s\n", levelIndex, levelPaths[levelIndex]);
		return false;
	}

	game->tilemap = &tm;
	float spawnX = 0.0f;
	float spawnY = 640.0f;
	if(findSpawnPoint(tm.map, &spawnX, &spawnY)){
		printf("Spawn point found: %.2f, %.2f\n", spawnX, spawnY);
	} else {
		printf("No spawn object found; using fallback spawn.\n");
	}
	resetPlayerSpawn(&player, spawnX, spawnY);
	game->cam.x = 0;
	game->cam.y = 0;
	game->cam.fx = 0.0f;
	game->cam.fy = 0.0f;
	currentLevel = levelIndex;
	printf("Loaded level %d: %s\n", currentLevel + 1, levelPaths[currentLevel]);
	return true;
}

static bool advanceToNextLevel(Game *game){
	if(currentLevel + 1 >= levelCount){
		printf("No more levels yet.\n");
		return false;
	}
	return loadLevel(game, currentLevel + 1);
}

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
		SDL_WINDOW_SHOWN);
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

	/* initialize camera */
	game->cam.w = SCREEN_WIDTH;
	game->cam.h = SCREEN_HEIGHT;
	game->cam.x = 0;
	game->cam.y = 0;
	game->cam.fx = 0.0f;
	game->cam.fy = 0.0f;
	game->cam.lerpFactor = 0.15f; /* default smoothing */
	game->cam.smoothingEnabled = true;
	game->cam.optimizeRender = true;
	game->showDebug = false;

	// OTHER INITIALIZATIONS HERE
	/* expose the tilemap to the game so other systems can query collisions */
	game->tilemap = &tm;
	playerInit(&player, game->renderer);
	if(!loadLevel(game, 0)){
		return -1;
	}

	// --------------------------
	// TTF ----------------------
	TTF_Init();

	game->font = (Font*)malloc(sizeof(Font));
	if(!game->font){
		fprintf(stderr, "Failed to allocate memory for font struct\n");
		return -1;
	}
	memset(game->font, 0, sizeof(Font));
	
	game->font->font = TTF_OpenFont("../assets/font.ttf", 48);

	loadScreenAssets(game);

	// ---------------------------
	
 	game->running = true;
 	game->state = START;

 	printf("Game initialized\n");
 	return 0;
}

void gameHandleEvent(Game* game){
	playerHandleInput(&player, game);
	SDL_Event event;
	// Process all pending SDL events and update keyboard state
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			game->running = false;
		}
		if(event.type == SDL_KEYDOWN){
			if(event.key.repeat == 0){
				if(event.key.keysym.scancode == SDL_SCANCODE_F1){
					/* toggle smoothing */
					game->cam.smoothingEnabled = !game->cam.smoothingEnabled;
					printf("Camera smoothing: %s\n", game->cam.smoothingEnabled ? "ON" : "OFF");
				}
				if(event.key.keysym.scancode == SDL_SCANCODE_F2){
					/* toggle debug overlays */
					game->showDebug = !game->showDebug;
					printf("Debug overlays: %s\n", game->showDebug ? "ON" : "OFF");
				}
				if(event.key.keysym.scancode == SDL_SCANCODE_F3){
					/* toggle render optimization */
					game->cam.optimizeRender = !game->cam.optimizeRender;
					printf("Render culling: %s\n", game->cam.optimizeRender ? "ON" : "OFF");
				}
				if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					game->state = GAME;
				}
			}
			if(event.key.keysym.scancode < SDL_NUM_SCANCODES)
				game->keys[event.key.keysym.scancode] = true;
		}
		if(event.type == SDL_KEYUP){
			if(event.key.keysym.scancode < SDL_NUM_SCANCODES)
				game->keys[event.key.keysym.scancode] = false;
		}
	}
}

void gameUpdate(Game* game, float dt){
	playerUpdate(&player, game, dt);

	SDL_Rect interactionRect = {(int)player.x, (int)player.y, player.width, player.height};
	bool interactKeyDown = game->keys[SDL_SCANCODE_E];
	if(interactKeyDown && !lastInteractKey && game->tilemap && tilemapTouchingDoor(game->tilemap, &interactionRect)){
		if(advanceToNextLevel(game)){
			lastInteractKey = interactKeyDown;
			return;
		}
	}
	lastInteractKey = interactKeyDown;

	// Update camera to follow player
	int mapW = SCREEN_WIDTH;
	int mapH = SCREEN_HEIGHT;
	if(game->tilemap && game->tilemap->map){
		mapW = game->tilemap->map->width * game->tilemap->map->tile_width;
		mapH = game->tilemap->map->height * game->tilemap->map->tile_height;
	}
	SDL_Rect pr = {(int)player.x, (int)player.y, player.width, player.height};
	updateCamera(&game->cam, pr, mapW, mapH);
}

void gameRender(Game *game){
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

		gameHandleEvent(game);

		switch (game->state) {
			case 0:
				startScreen(game);
				SDL_RenderPresent(game->renderer);
				break;
			case 1:
				gameRender(game);
				gameUpdate(game, deltaTime);
				break;
			case 2:
				endScreen(game);
				SDL_RenderPresent(game->renderer);
				break;
			}

			SDL_Delay(16);
 	}
}

void gameClean(Game *game){
	tilemapClean(&tm);
	playerClean(&player);
	animatorFree(&a);
	if(game->font){
		free(game->font);
		game->font = NULL;
	}
	SDL_DestroyWindow(game->window);
	SDL_DestroyRenderer(game->renderer);
	IMG_Quit();
	SDL_Quit();
}
