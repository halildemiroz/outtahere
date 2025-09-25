#ifndef TILEMAP_H
#define TILEMAP_H

#include <SDL2/SDL.h>

#ifdef HAVE_TMX
#include "tmx.h"

typedef struct {
    tmx_map *map;
    SDL_Texture **tilesets;
    int tileset_count;
} Tilemap;

#else
typedef struct {
    void *map;
    void **tilesets;
    int tileset_count;
} Tilemap;
#endif

int tilemap_init(Tilemap *tilemap, const char *filename, SDL_Renderer *renderer);
void tilemap_cleanup(Tilemap *tilemap);

#endif /* TILEMAP_H */