#ifndef TILEMAP_H
#define TILEMAP_H

#include <SDL2/SDL.h>
#include <tmx.h>
#include <stdbool.h>

#include "camera.h"

/* Default list of substrings in layer names that should be ignored for collisions.
   You can override this by defining `TILEMAP_IGNORE_LAYER_NAMES` before including this header.
   Example: #define TILEMAP_IGNORE_LAYER_NAMES {"background","bg","decoration","parallax"}
*/
#ifndef TILEMAP_IGNORE_LAYER_NAMES
#define TILEMAP_IGNORE_LAYER_NAMES {"background", "bg", "decoration"}
#endif

typedef struct {
	int pointCount;
	float* points; /* x0,y0, x1,y1 ... */
} TilePolygon;

typedef struct {
	SDL_Rect rect;
	/* optional polygon override (local coordinates relative to object.x/object.y) */
	TilePolygon polygon;
	bool hasPolygon;
} TileObject;

typedef struct Tilemap {
	tmx_map* map;
	SDL_Texture** tileset;
	int tilesetCount;
} Tilemap;

int tilemapInit(Tilemap* tm, const char* filename, SDL_Renderer* renderer);

void tilemapClean(Tilemap* tm);
void tilemapRender(Tilemap* tm, SDL_Renderer* renderer, Camera* cam);
void tilemapDebugRender(Tilemap* tm, SDL_Renderer* renderer, SDL_Rect* playerRect, Camera* cam);
bool tilemapCheckCollision(Tilemap* tm, SDL_Rect* rect);

/* removes collectible tiles that intersect `rect`; returns true if any were collected */
bool tilemapCollectCollectibles(Tilemap* tm, SDL_Rect* rect, int* collectedCount);

/* returns a pointer to a collision object that intersects `rect`, or NULL */
TileObject* tilemapGetCollisionObject(Tilemap* tm, SDL_Rect* rect);

/* compute minimal translation vector (MTV) between a polygon object and rect
   returns 1 if intersecting and sets (nx,ny) normalized and overlap > 0
   returns 0 if not intersecting */
int tilemapPolygonRectMTV(const TileObject* to, const SDL_Rect* r, float* nx, float* ny, float* overlap);

#endif
