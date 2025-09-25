#include <stdio.h>
#include <SDL2/SDL.h>
#include "tilemap.h"

#ifdef HAVE_TMX
#include "tmx.h"

static void tilemap_error_handler(const char *err) {
    fprintf(stderr, "TMX Error: %s\n", err);
}
#endif

int tilemap_init(Tilemap *tilemap, const char *filename, SDL_Renderer *renderer) {
#ifdef HAVE_TMX
    tilemap->map = NULL;
    tilemap->tilesets = NULL;
    tilemap->tileset_count = 0;

    // Set error handler
    tmx_set_error_handler(tilemap_error_handler);
    
    // Load the map
    tilemap->map = tmx_load(filename);
    if (!tilemap->map) {
        return -1;
    }
    
    return 0;
#else
    return -1;
#endif
}

void tilemap_cleanup(Tilemap *tilemap) {
#ifdef HAVE_TMX
    if (tilemap->map) {
        tmx_map_free(tilemap->map);
        tilemap->map = NULL;
    }
    if (tilemap->tilesets) {
        free(tilemap->tilesets);
        tilemap->tilesets = NULL;
    }
    tilemap->tileset_count = 0;
#endif
}