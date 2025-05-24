#pragma once
#include <tmx.h>
#include <stdio.h>
#include "game.h"
#include "tile.h"

typedef struct {
    tmx_layer* layer;
    tmx_map* map;
} Map;
extern Map map;

void mapLoad(const char* path);
void mapRender();
void layerRender(tmx_map* map, tmx_layer* layer);
void mapFree();


