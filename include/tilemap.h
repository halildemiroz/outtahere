#ifndef TILEMAP_H
#define TILEMAP_H

#include <game.h>

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

	/* collision objects loaded from object layers */
	TileObject* objects;
	int objectCount;

	/* optional per-gid polygon overrides parsed from tileset <tile> collision shapes
	   indexed by global gid (0..gidPolygonCount-1). A pointCount==0 means no polygon. */
	TilePolygon* gidPolygons;
	int gidPolygonCount;
} Tilemap;

int tilemapInit(Tilemap* tm, const char* filename, SDL_Renderer* renderer);

void tilemapClean(Tilemap* tm);
void tilemapRender(Tilemap* tm, SDL_Renderer* renderer);
void tilemapDebugRender(Tilemap* tm, SDL_Renderer* renderer, SDL_Rect* playerRect);
bool tilemapCheckCollision(Tilemap* tm, SDL_Rect* rect);

/* returns a pointer to a collision object that intersects `rect`, or NULL */
TileObject* tilemapGetCollisionObject(Tilemap* tm, SDL_Rect* rect);

/* compute minimal translation vector (MTV) between a polygon object and rect
   returns 1 if intersecting and sets (nx,ny) normalized and overlap > 0
   returns 0 if not intersecting */
int tilemapPolygonRectMTV(const TileObject* to, const SDL_Rect* r, float* nx, float* ny, float* overlap);

/* returns true if the given tile coordinate contains a non-zero gid */
bool tilemapIsSolidAt(Tilemap* tm, int tx, int ty);

#endif
