#include <tilemap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/* Simple polygon collision checking for tiles */

/* Check if a point is inside a polygon using ray casting */
static int pointInPolygon(float x, float y, const float* points, int pointCount) {
    int i, j, c = 0;
    for (i = 0, j = pointCount - 1; i < pointCount; j = i++) {
        float xi = points[i*2], yi = points[i*2+1];
        float xj = points[j*2], yj = points[j*2+1];
        
        if (((yi > y) != (yj > y)) && 
            (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            c = !c;
        }
    }
    return c;
}

/* Check if a rectangle intersects with a polygon */
int polygonRectCollision(float tileX, float tileY, const float* points, int pointCount, const SDL_Rect* rect) {
    if (!points || pointCount <= 0) return 0;
    
    /* Check if any corner of the rectangle is inside the polygon */
    float corners[4][2] = {
        {(float)rect->x, (float)rect->y},
        {(float)(rect->x + rect->w), (float)rect->y},
        {(float)(rect->x + rect->w), (float)(rect->y + rect->h)},
        {(float)rect->x, (float)(rect->y + rect->h)}
    };
    
    /* Transform polygon points to world coordinates */
    float* worldPoints = malloc(pointCount * 2 * sizeof(float));
    for (int i = 0; i < pointCount; i++) {
        worldPoints[i*2] = tileX + points[i*2];
        worldPoints[i*2+1] = tileY + points[i*2+1];
    }
    
    /* Check each corner */
    for (int i = 0; i < 4; i++) {
        if (pointInPolygon(corners[i][0], corners[i][1], worldPoints, pointCount)) {
            free(worldPoints);
            return 1;
        }
    }
    
    /* Check if polygon center is inside rectangle */
    float cx = 0, cy = 0;
    for (int i = 0; i < pointCount; i++) {
        cx += worldPoints[i*2];
        cy += worldPoints[i*2+1];
    }
    cx /= pointCount;
    cy /= pointCount;
    
    if (cx >= rect->x && cx <= rect->x + rect->w && 
        cy >= rect->y && cy <= rect->y + rect->h) {
        free(worldPoints);
        return 1;
    }
    
    free(worldPoints);
    return 0;
}

/* Parse a simple polygon string like "0,0 31.6364,-31.9992 31.8182,0.0916883" */
int parsePolygonPoints(const char* pointsStr, float** outPoints, int* outCount) {
    if (!pointsStr || !outPoints || !outCount) return 0;
    
    int capacity = 8;
    float* points = malloc(capacity * 2 * sizeof(float));
    int count = 0;
    
    char* str = strdup(pointsStr);
    char* token = strtok(str, " ");
    
    while (token != NULL) {
        float x, y;
        if (sscanf(token, "%f,%f", &x, &y) == 2) {
            if (count >= capacity) {
                capacity *= 2;
                points = realloc(points, capacity * 2 * sizeof(float));
            }
            points[count*2] = x;
            points[count*2+1] = y;
            count++;
        }
        token = strtok(NULL, " ");
    }
    
    free(str);
    
    if (count > 0) {
        *outPoints = points;
        *outCount = count;
        return 1;
    } else {
        free(points);
        *outPoints = NULL;
        *outCount = 0;
        return 0;
    }
}