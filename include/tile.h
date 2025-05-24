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

GIDResult findTileset(tmx_map* map, unsigned int gid);

