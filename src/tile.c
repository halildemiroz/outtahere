#include "../include/tile.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_surface.h"

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

