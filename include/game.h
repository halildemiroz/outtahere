#ifndef GAME_H
#define GAME_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <tmx.h>
#include <stdbool.h>
#include "camera.h"

#define SCREEN_WIDTH 920
#define SCREEN_HEIGHT 640
#define TILE_SIZE 16

typedef struct{
    TTF_Font* font;
    SDL_Surface* startSurface;
    SDL_Surface* endSurface;
    SDL_Texture* startTexture;
    SDL_Texture* endTexture;
} Font;

typedef enum {
    START,
    GAME,
    END
} GameState;

/* forward declaration to avoid circular includes */
typedef struct Tilemap Tilemap;

typedef struct {
 	SDL_Window* window;
 	SDL_Renderer* renderer;
 	bool running;
 	bool keys[SDL_NUM_SCANCODES];
 	Tilemap* tilemap;
    Font* font;
 	Camera cam;
 	bool showDebug; /* toggle debug overlays */
 	GameState state;
 } 	Game;
extern Game game;

int gameInit(Game *game, const char* name);

void gameClean(Game* game);
void gameHandleEvent(Game* game);
void gameUpdate(Game* game, float dt);
void gameRender(Game* game);
void gameRun(Game* game);

#endif

