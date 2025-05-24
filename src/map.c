#include "../include/map.h"
#include "SDL2/SDL_render.h"
#include <stdio.h>

Map map;

void mapLoad(const char* path){
    map.map = tmx_load(path);
    if (!map.map)
        fprintf(stderr, "Failed to load TMX map: %s\n", tmx_strerr());
}

void layerRender(tmx_map *map, tmx_layer *layer){
    if(!map || !layer || layer->type != L_LAYER || !layer->visible) return;

    unsigned int i, j;
    for(i = 0; i < map->height; i++){
        for(j = 0; j < map->width; j++){
            unsigned int gid = layer->content.gids[(i*map->width) + j];
            if(gid==0) continue;
            
            GIDResult res = findTileset(map, gid);
            if(!res.tileset) continue;
            
            unsigned int local_id = gid -  res.firstgid;
            SDL_Texture* tilesetTexture = (SDL_Texture*)res.tileset->image->resource_image;
            if(!tilesetTexture)fprintf(stderr, "Tileset texture is NULL\n");
            
            unsigned int tw = map->tile_width;
            unsigned int th = map->tile_height;
            unsigned int tilesPerRow = res.tileset->image->width / tw;

            SDL_Rect src = {
                (local_id % tilesPerRow) * tw,
                (local_id / tilesPerRow) * th,
                tw, th
            };
            SDL_Rect dest = {
                j * tw,
                i * th,
                tw, th
            };
            SDL_RenderCopy(game.renderer, tilesetTexture, &src, &dest);
        }
    }
}

void mapRender(){
    SDL_RenderClear(game.renderer);
    map.layer = map.map->ly_head;
    while(map.layer){
       layerRender(map.map, map.layer);
       map.layer = map.layer->next;
    }
    //SDL_RenderPresent(game.renderer);
    SDL_Delay(16);
}

void mapFree(){
    tmx_map_free(map.map);
}

