#include "tilemap.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_TMX
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
    
    // Load the TMX map
    tilemap->map = tmx_load(filename);
    if (!tilemap->map) {
        fprintf(stderr, "Failed to load tilemap: %s\n", filename);
        return -1;
    }
    
    printf("Loaded tilemap: %s (%dx%d tiles, tile size: %dx%d)\n",
           filename,
           tilemap->map->width, tilemap->map->height,
           tilemap->map->tile_width, tilemap->map->tile_height);
    
    // Load tilesets (simplified for now)
    // In a full implementation, you'd load all tileset images here
    
    return 0;
#else
    printf("Tilemap functionality disabled (libtmx not available)\n");
    tilemap->dummy = 0;
    return 0;
#endif
}

void tilemap_cleanup(Tilemap *tilemap) {
#ifdef HAVE_TMX
    if (tilemap->tilesets) {
        for (int i = 0; i < tilemap->tileset_count; i++) {
            if (tilemap->tilesets[i]) {
                SDL_DestroyTexture(tilemap->tilesets[i]);
            }
        }
        free(tilemap->tilesets);
        tilemap->tilesets = NULL;
    }
    
    if (tilemap->map) {
        tmx_map_free(tilemap->map);
        tilemap->map = NULL;
    }
#endif
}

void tilemap_render(Tilemap *tilemap, SDL_Renderer *renderer) {
#ifdef HAVE_TMX
    if (!tilemap->map) return;
    
    // This is a simplified render function
    // A full implementation would render all layers and handle tilesets properly
    
    tmx_layer *layer = tilemap->map->ly_head;
    while (layer) {
        if (layer->visible && layer->type == L_LAYER) {
            // Render tiles for this layer
            for (int y = 0; y < tilemap->map->height; y++) {
                for (int x = 0; x < tilemap->map->width; x++) {
                    unsigned int gid = layer->content.gids[(y * tilemap->map->width) + x];
                    if (gid != 0) {
                        // For now, just draw colored rectangles for tiles
                        SDL_Rect tile_rect = {
                            x * tilemap->map->tile_width,
                            y * tilemap->map->tile_height,
                            tilemap->map->tile_width,
                            tilemap->map->tile_height
                        };
                        
                        // Different colors for different tile IDs
                        if (gid == 1) {
                            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown for ground
                        } else {
                            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray for other tiles
                        }
                        
                        SDL_RenderFillRect(renderer, &tile_rect);
                    }
                }
            }
        }
        layer = layer->next;
    }
#else
    // Simple ground rendering without TMX
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown ground
    SDL_Rect ground_rect = {0, 536, 800, 64}; // Ground at bottom of screen
    SDL_RenderFillRect(renderer, &ground_rect);
#endif
}

bool tilemap_check_collision(Tilemap *tilemap, SDL_Rect *rect) {
#ifdef HAVE_TMX
    if (!tilemap->map) return false;
    
    // Simple collision detection with solid tiles
    int start_x = rect->x / tilemap->map->tile_width;
    int start_y = rect->y / tilemap->map->tile_height;
    int end_x = (rect->x + rect->w - 1) / tilemap->map->tile_width;
    int end_y = (rect->y + rect->h - 1) / tilemap->map->tile_height;
    
    tmx_layer *layer = tilemap->map->ly_head;
    while (layer) {
        if (layer->visible && layer->type == L_LAYER) {
            for (int y = start_y; y <= end_y; y++) {
                for (int x = start_x; x <= end_x; x++) {
                    if (x >= 0 && x < tilemap->map->width && y >= 0 && y < tilemap->map->height) {
                        unsigned int gid = layer->content.gids[(y * tilemap->map->width) + x];
                        if (gid != 0) {
                            return true; // Collision detected
                        }
                    }
                }
            }
        }
        layer = layer->next;
    }
    
    return false;
#else
    // Simple ground collision check
    return (rect->y + rect->h >= 536); // Ground level
#endif
}