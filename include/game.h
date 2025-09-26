#ifndef GAME_H
#define GAME_H
#include "SDL_scancode.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <tmx.h>
#include <stdbool.h>

#define SCREEN_WIDTH 920
#define SCREEN_HEIGHT 640
#define TILE_SIZE 16

/* forward declaration to avoid circular includes */
typedef struct Tilemap Tilemap;

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool running;
	bool keys[SDL_NUM_SCANCODES];
	Tilemap* tilemap;
} 	Game;
extern Game game;

int gameInit(Game *game, const char* name);
void gameClean(Game* game);
void gameHandleEvent(Game* game);
void gameUpdate(Game* game, float dt);
void gameRender(Game* game);
void gameRun(Game* game);

#endif

