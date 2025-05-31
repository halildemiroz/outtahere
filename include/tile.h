#pragma once

#include "game.h"
#include <tmx.h>
#include <SDL2/SDL_image.h>

typedef struct {
    tmx_tileset *tileset;
    unsigned int firstgid;
} GIDResult;

void* textureLoader(const char* path);
void textureFree(void* ptr);

void tileInit();

bool isSolidTile(unsigned int gid, tmx_map* map);
bool checkPointCollision(tmx_map* map, int x, int y);
bool checkSpriteCollision(tmx_map *map, int x, int y, int width, int height);

GIDResult findTileset(tmx_map* map, unsigned int gid);

