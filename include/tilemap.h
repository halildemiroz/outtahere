#ifndef TILEMAP_H
#define TILEMAP_H

#include <game.h>

typedef struct{
	tmx_map* map;
	SDL_Texture** tileset;
	int tilesetCount;
} Tilemap;

int tilemapInit(Tilemap* tm, const char* filename, SDL_Renderer* renderer);

void tilemapClean(Tilemap* tm);
void tilemapRender(Tilemap* tm, SDL_Renderer* renderer);
bool tilemapCheckCollision(Tilemap* tm, SDL_Rect* rect);

#endif
