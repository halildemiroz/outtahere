#include "tmx.h"
#include <stdio.h>
#include <stdlib.h>

// Static error handler function pointer
static void (*error_handler)(const char *) = NULL;

void tmx_set_error_handler(void (*handler)(const char *)) {
    error_handler = handler;
}

tmx_map* tmx_load(const char *filename) {
    // For testing purposes, create a dummy map
    tmx_map *map = malloc(sizeof(tmx_map));
    if (!map) {
        if (error_handler) {
            error_handler("Failed to allocate memory for map");
        }
        return NULL;
    }
    
    map->dummy = 42;
    printf("TMX: Loading map from %s (stub implementation)\n", filename);
    return map;
}

void tmx_map_free(tmx_map *map) {
    if (map) {
        printf("TMX: Freeing map (stub implementation)\n");
        free(map);
    }
}