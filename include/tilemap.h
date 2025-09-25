#ifndef TILEMAP_H
#define TILEMAP_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#ifdef HAVE_TMX
#include <tmx.h>
#endif

typedef struct {
#ifdef HAVE_TMX
    tmx_map *map;
    SDL_Texture **tilesets;
    int tileset_count;
#else
    int dummy; // Placeholder when TMX is not available
#endif
} Tilemap;

// Tilemap functions
int tilemap_init(Tilemap *tilemap, const char *filename, SDL_Renderer *renderer);
void tilemap_cleanup(Tilemap *tilemap);
void tilemap_render(Tilemap *tilemap, SDL_Renderer *renderer);
bool tilemap_check_collision(Tilemap *tilemap, SDL_Rect *rect);

#endif // TILEMAP_H