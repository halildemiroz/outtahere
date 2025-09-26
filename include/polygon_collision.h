#ifndef POLYGON_COLLISION_H
#define POLYGON_COLLISION_H

#include <SDL2/SDL.h>

/* Check if a rectangle intersects with a polygon at given tile position */
int polygonRectCollision(float tileX, float tileY, const float* points, int pointCount, const SDL_Rect* rect);

/* Parse polygon points from string format like "0,0 31.6364,-31.9992 31.8182,0.0916883" */
int parsePolygonPoints(const char* pointsStr, float** outPoints, int* outCount);

#endif