#include "../include/tile.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_surface.h"
#include "tmx.h"

void* textureLoader(const char* path){
    SDL_Surface* surface = IMG_Load(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game.renderer, surface);
    SDL_FreeSurface(surface);

    return texture;
}

void textureFree(void* ptr){
    SDL_DestroyTexture((SDL_Texture*)ptr);
}

void tileInit(){
    tmx_img_load_func = textureLoader;
    tmx_img_free_func = textureFree;
}

GIDResult findTileset(tmx_map* map, unsigned int gid){
    GIDResult result = {NULL, 0};
    
    for(tmx_tileset_list* tsl = map->ts_head; tsl != NULL; tsl = tsl->next){
        if(gid >= tsl->firstgid){
            if(!result.tileset || tsl->firstgid > result.firstgid){
                result.tileset = tsl->tileset;
                result.firstgid = tsl->firstgid;
            }
        }
    }
    return result;
}

bool isSolidTile(unsigned int gid, tmx_map* map){
    tmx_tile* tile = tmx_get_tile(map, gid);
    if(tile && tile->properties){
        tmx_property* prop = tmx_get_property(tile->properties, "solid");
        if(prop && prop->type == PT_STRING && strcmp(prop->value.string, "true") == 0){
            return true;
        }
    }
    return false;
}

bool checkPointCollision(tmx_map *map, int x, int y){
    int tileX = x / map->tile_width;
    int tileY = y / map->tile_height;

    if(tileX < 0 || tileX >= map->width || tileY < 0 || tileY >= map->height)
        return false;
    for(tmx_layer* layer = map->ly_head; layer; layer = layer->next){
        if(layer->visible && layer->type == L_LAYER){
            unsigned int gid = layer->content.gids[(tileY * map->width) + tileX];
            if(gid != 0 && isSolidTile(gid, map))
                return true;
        }
    }
    return false;
}

bool checkSpriteCollision(tmx_map *map, int x, int y, int width, int height) {
    int startTileX = x / map->tile_width;
    int startTileY = y / map->tile_height;
    int endTileX = (x + width - 1) / map->tile_width;
    int endTileY = (y + height - 1) / map->tile_height;

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        for (int tileX = startTileX; tileX <= endTileX; tileX++) {
            if (checkPointCollision(map, tileX * map->tile_width, tileY * map->tile_height)) {
                return true;
            }
        }
    }
    return false;
}
