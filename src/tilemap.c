#include "tilemap.h"
#include "camera.h"
#include <polygon_collision.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TileObject collisionHitCache = {0};

static void clearTileObject(TileObject *obj){
	if(!obj) return;
	if(obj->hasPolygon && obj->polygon.points){
		free(obj->polygon.points);
	}
	memset(obj, 0, sizeof(*obj));
}

static bool propertyValueToBool(const tmx_property *prop){
	if(!prop) return false;

	switch(prop->type){
		case PT_BOOL:
			return prop->value.boolean != 0;
		case PT_INT:
			return prop->value.integer != 0;
		case PT_FLOAT:
			return fabsf(prop->value.decimal) > 0.00001f;
		case PT_STRING: {
			const char *s = prop->value.string;
			if(!s) return false;
			while(*s && isspace((unsigned char)*s)) s++;
			if(!*s) return false;
			char buf[32];
			size_t i = 0;
			while(s[i] && i < sizeof(buf) - 1){
				buf[i] = (char)tolower((unsigned char)s[i]);
				i++;
			}
			buf[i] = '\0';
			return strcmp(buf, "1") == 0 || strcmp(buf, "true") == 0 || strcmp(buf, "yes") == 0 || strcmp(buf, "on") == 0 || strcmp(buf, "collectible") == 0 || strcmp(buf, "coin") == 0;
		}
		default:
			return false;
	}
}

static bool tileHasPropertyBool(const tmx_tile *tile, const char *name){
	if(!tile || !name || !tile->properties) return false;
	return propertyValueToBool(tmx_get_property(tile->properties, name));
}

static bool tileIsCollectible(const tmx_tile *tile){
	return tileHasPropertyBool(tile, "collectible") || tileHasPropertyBool(tile, "collectable") || tileHasPropertyBool(tile, "coin");
}

static bool tileIsDoor(const tmx_tile *tile){
	return tileHasPropertyBool(tile, "door");
}

static bool layerShouldBeIgnored(const char *name){
	if(!name) return false;
	const char *ignored[] = {"background", "bg", "decoration", "spawn"};
	for(size_t i = 0; i < sizeof(ignored) / sizeof(ignored[0]); i++){
		if(strstr(name, ignored[i])) return true;
	}
	return false;
}

static bool rectsIntersect(const SDL_Rect *a, const SDL_Rect *b){
	return a && b && SDL_HasIntersection(a, b) == SDL_TRUE;
}

static void projectPointsOnAxis(const float *points, int pointCount, float axisX, float axisY, float *minOut, float *maxOut){
	float minV = points[0] * axisX + points[1] * axisY;
	float maxV = minV;

	for(int i = 1; i < pointCount; i++){
		float p = points[i * 2] * axisX + points[i * 2 + 1] * axisY;
		if(p < minV) minV = p;
		if(p > maxV) maxV = p;
	}

	if(minOut) *minOut = minV;
	if(maxOut) *maxOut = maxV;
}

static void projectRectOnAxis(const SDL_Rect *rect, float axisX, float axisY, float *minOut, float *maxOut){
	float corners[4][2] = {
		{(float)rect->x, (float)rect->y},
		{(float)(rect->x + rect->w), (float)rect->y},
		{(float)(rect->x + rect->w), (float)(rect->y + rect->h)},
		{(float)rect->x, (float)(rect->y + rect->h)}
	};

	float minV = corners[0][0] * axisX + corners[0][1] * axisY;
	float maxV = minV;

	for(int i = 1; i < 4; i++){
		float p = corners[i][0] * axisX + corners[i][1] * axisY;
		if(p < minV) minV = p;
		if(p > maxV) maxV = p;
	}

	if(minOut) *minOut = minV;
	if(maxOut) *maxOut = maxV;
}

static bool polygonRectSAT(const float *points, int pointCount, const SDL_Rect *rect, float *nx, float *ny, float *overlap){
	if(!points || !rect || pointCount < 3) return false;

	float polyCx = 0.0f;
	float polyCy = 0.0f;
	for(int i = 0; i < pointCount; i++){
		polyCx += points[i * 2];
		polyCy += points[i * 2 + 1];
	}
	polyCx /= (float)pointCount;
	polyCy /= (float)pointCount;

	float rectCx = (float)rect->x + ((float)rect->w * 0.5f);
	float rectCy = (float)rect->y + ((float)rect->h * 0.5f);
	float centerDx = rectCx - polyCx;
	float centerDy = rectCy - polyCy;

	float bestOverlap = FLT_MAX;
	float bestNx = 0.0f;
	float bestNy = 0.0f;

	for(int i = 0; i < pointCount; i++){
		int j = (i + 1) % pointCount;
		float edgeX = points[j * 2] - points[i * 2];
		float edgeY = points[j * 2 + 1] - points[i * 2 + 1];
		float axisX = -edgeY;
		float axisY = edgeX;
		float axisLen = sqrtf(axisX * axisX + axisY * axisY);

		if(axisLen < 0.00001f) continue;
		axisX /= axisLen;
		axisY /= axisLen;

		if(centerDx * axisX + centerDy * axisY < 0.0f){
			axisX = -axisX;
			axisY = -axisY;
		}

		float polyMin, polyMax, rectMin, rectMax;
		projectPointsOnAxis(points, pointCount, axisX, axisY, &polyMin, &polyMax);
		projectRectOnAxis(rect, axisX, axisY, &rectMin, &rectMax);

		float o = fminf(polyMax, rectMax) - fmaxf(polyMin, rectMin);
		if(o <= 0.0f) return false;

		if(o < bestOverlap){
			bestOverlap = o;
			bestNx = axisX;
			bestNy = axisY;
		}
	}

	/* add the rectangle axes too */
	for(int axisIndex = 0; axisIndex < 2; axisIndex++){
		float axisX = (axisIndex == 0) ? 1.0f : 0.0f;
		float axisY = (axisIndex == 0) ? 0.0f : 1.0f;

		if(centerDx * axisX + centerDy * axisY < 0.0f){
			axisX = -axisX;
			axisY = -axisY;
		}

		float polyMin, polyMax, rectMin, rectMax;
		projectPointsOnAxis(points, pointCount, axisX, axisY, &polyMin, &polyMax);
		projectRectOnAxis(rect, axisX, axisY, &rectMin, &rectMax);

		float o = fminf(polyMax, rectMax) - fmaxf(polyMin, rectMin);
		if(o <= 0.0f) return false;

		if(o < bestOverlap){
			bestOverlap = o;
			bestNx = axisX;
			bestNy = axisY;
		}
	}

	if(nx) *nx = bestNx;
	if(ny) *ny = bestNy;
	if(overlap) *overlap = bestOverlap;
	return true;
}

static bool rectRectMTV(const SDL_Rect *a, const SDL_Rect *b, float *nx, float *ny, float *overlap){
	if(!a || !b || !rectsIntersect(a, b)) return false;

	float aLeft = (float)a->x;
	float aRight = (float)(a->x + a->w);
	float aTop = (float)a->y;
	float aBottom = (float)(a->y + a->h);

	float bLeft = (float)b->x;
	float bRight = (float)(b->x + b->w);
	float bTop = (float)b->y;
	float bBottom = (float)(b->y + b->h);

	float overlapLeft = aRight - bLeft;
	float overlapRight = bRight - aLeft;
	float overlapTop = aBottom - bTop;
	float overlapBottom = bBottom - aTop;

	float minOverlap = overlapLeft;
	float outNx = 1.0f;
	float outNy = 0.0f;

	if(overlapRight < minOverlap){
		minOverlap = overlapRight;
		outNx = -1.0f;
		outNy = 0.0f;
	}
	if(overlapTop < minOverlap){
		minOverlap = overlapTop;
		outNx = 0.0f;
		outNy = 1.0f;
	}
	if(overlapBottom < minOverlap){
		minOverlap = overlapBottom;
		outNx = 0.0f;
		outNy = -1.0f;
	}

	float aCx = (float)a->x + ((float)a->w * 0.5f);
	float aCy = (float)a->y + ((float)a->h * 0.5f);
	float bCx = (float)b->x + ((float)b->w * 0.5f);
	float bCy = (float)b->y + ((float)b->h * 0.5f);
	float dx = bCx - aCx;
	float dy = bCy - aCy;

	if(fabsf(outNx) > fabsf(outNy)){
		if(dx < 0.0f) outNx = -fabsf(outNx);
		else outNx = fabsf(outNx);
	} else {
		if(dy < 0.0f) outNy = -fabsf(outNy);
		else outNy = fabsf(outNy);
	}

	if(nx) *nx = outNx;
	if(ny) *ny = outNy;
	if(overlap) *overlap = minOverlap;
	return true;
}

static bool buildCollisionObject(const tmx_object *obj, float baseX, float baseY, int fallbackW, int fallbackH, TileObject *out){
	if(!obj || !out) return false;

	clearTileObject(out);

	float originX = baseX + (float)obj->x;
	float originY = baseY + (float)obj->y;

	if(obj->obj_type == OT_POLYGON && obj->content.shape && obj->content.shape->points_len > 0){
		int count = obj->content.shape->points_len;
		float *points = (float*)malloc((size_t)count * 2 * sizeof(float));
		if(!points) return false;

		float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
		for(int i = 0; i < count; i++){
			float px = originX + (float)obj->content.shape->points[i][0];
			float py = originY + (float)obj->content.shape->points[i][1];
			points[i * 2] = px;
			points[i * 2 + 1] = py;
			if(i == 0){
				minX = maxX = px;
				minY = maxY = py;
			} else {
				if(px < minX) minX = px;
				if(px > maxX) maxX = px;
				if(py < minY) minY = py;
				if(py > maxY) maxY = py;
			}
		}

		out->rect.x = (int)floorf(minX);
		out->rect.y = (int)floorf(minY);
		out->rect.w = (int)ceilf(maxX - minX);
		out->rect.h = (int)ceilf(maxY - minY);
		if(out->rect.w <= 0) out->rect.w = 1;
		if(out->rect.h <= 0) out->rect.h = 1;
		out->polygon.pointCount = count;
		out->polygon.points = points;
		out->hasPolygon = true;
		return true;
	}

	int w = obj->width > 0.0 ? (int)ceilf((float)obj->width) : fallbackW;
	int h = obj->height > 0.0 ? (int)ceilf((float)obj->height) : fallbackH;
	if(w <= 0) w = 1;
	if(h <= 0) h = 1;

	out->rect.x = (int)floorf(originX);
	out->rect.y = (int)floorf(originY);
	out->rect.w = w;
	out->rect.h = h;
	out->hasPolygon = false;
	return true;
}

static bool collisionObjectIntersectsRect(const TileObject *obj, const SDL_Rect *rect){
	if(!obj || !rect) return false;
	if(obj->hasPolygon && obj->polygon.points && obj->polygon.pointCount >= 3){
		return polygonRectSAT(obj->polygon.points, obj->polygon.pointCount, rect, NULL, NULL, NULL);
	}
	return rectsIntersect(&obj->rect, rect);
}

static bool tileIntersectsRect(const tmx_tile *tile, int tileX, int tileY, int tw, int th, const SDL_Rect *rect){
	if(!tile || !rect) return false;

	if(tile->collision){
		for(tmx_object *obj = tile->collision; obj; obj = obj->next){
			if(!obj->visible) continue;
			TileObject candidate = {0};
			if(!buildCollisionObject(obj, (float)tileX, (float)tileY, tw, th, &candidate)) continue;
			bool hit = collisionObjectIntersectsRect(&candidate, rect);
			clearTileObject(&candidate);
			if(hit) return true;
		}
		return false;
	}

	SDL_Rect tileRect = {tileX, tileY, tw, th};
	return rectsIntersect(&tileRect, rect);
}

static bool findCollisionInLayerList(Tilemap *tm, tmx_layer *layer, const SDL_Rect *rect, TileObject *outHit){
	while(layer){
		if(!layer->visible){
			layer = layer->next;
			continue;
		}

		if(layer->type == L_GROUP){
			if(findCollisionInLayerList(tm, layer->content.group_head, rect, outHit)) return true;
			layer = layer->next;
			continue;
		}

		if(layer->type == L_LAYER && !layerShouldBeIgnored(layer->name)){
			int tw = (int)tm->map->tile_width;
			int th = (int)tm->map->tile_height;
			float ox = (float)layer->offsetx;
			float oy = (float)layer->offsety;

			int startX = (int)floorf((((float)rect->x) - ox) / (float)tw) - 1;
			int startY = (int)floorf((((float)rect->y) - oy) / (float)th) - 1;
			int endX = (int)floorf((((float)(rect->x + rect->w - 1)) - ox) / (float)tw) + 1;
			int endY = (int)floorf((((float)(rect->y + rect->h - 1)) - oy) / (float)th) + 1;

			if(startX < 0) startX = 0;
			if(startY < 0) startY = 0;
			if(endX >= (int)tm->map->width) endX = (int)tm->map->width - 1;
			if(endY >= (int)tm->map->height) endY = (int)tm->map->height - 1;

			for(int y = startY; y <= endY; y++){
				for(int x = startX; x <= endX; x++){
					unsigned gid = layer->content.gids[(y * tm->map->width) + x] & TMX_FLIP_BITS_REMOVAL;
					if(gid == 0) continue;

					int tileX = (x * tw) + layer->offsetx;
					int tileY = (y * th) + layer->offsety;
					tmx_tile *tile = (gid < tm->map->tilecount) ? tm->map->tiles[gid] : NULL;
					if(tileIsCollectible(tile) || tileIsDoor(tile)) continue;

					if(tile && tile->collision){
						for(tmx_object *obj = tile->collision; obj; obj = obj->next){
							if(!obj->visible) continue;
							TileObject candidate = {0};
							if(!buildCollisionObject(obj, (float)tileX, (float)tileY, tw, th, &candidate)) continue;
							bool hit = collisionObjectIntersectsRect(&candidate, rect);
							if(hit){
								if(outHit){
									clearTileObject(outHit);
									*outHit = candidate;
								} else {
									clearTileObject(&candidate);
								}
								return true;
							}
							clearTileObject(&candidate);
						}
					} else {
						SDL_Rect tileRect = {tileX, tileY, tw, th};
						if(rectsIntersect(&tileRect, rect)){
							if(outHit){
								clearTileObject(outHit);
								outHit->rect = tileRect;
								outHit->hasPolygon = false;
							}
							return true;
						}
					}
				}
			}
		} else if(layer->type == L_OBJGR && layer->content.objgr && !layerShouldBeIgnored(layer->name)){
			float baseX = (float)layer->offsetx;
			float baseY = (float)layer->offsety;
			int fallbackW = (int)tm->map->tile_width;
			int fallbackH = (int)tm->map->tile_height;

			for(tmx_object *obj = layer->content.objgr->head; obj; obj = obj->next){
				if(!obj->visible) continue;
				TileObject candidate = {0};
				if(!buildCollisionObject(obj, baseX, baseY, fallbackW, fallbackH, &candidate)) continue;
				bool hit = collisionObjectIntersectsRect(&candidate, rect);
				if(hit){
					if(outHit){
						clearTileObject(outHit);
						*outHit = candidate;
					} else {
						clearTileObject(&candidate);
					}
					return true;
				}
				clearTileObject(&candidate);
			}
		}

		layer = layer->next;
	}

	return false;
}

static void drawTileObject(SDL_Renderer *renderer, const TileObject *obj, Camera *cam, SDL_Color color){
	if(!renderer || !obj) return;

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	int camX = cam ? cam->x : 0;
	int camY = cam ? cam->y : 0;

	if(obj->hasPolygon && obj->polygon.points && obj->polygon.pointCount >= 2){
		for(int i = 0; i < obj->polygon.pointCount; i++){
			int j = (i + 1) % obj->polygon.pointCount;
			int ax = (int)lroundf(obj->polygon.points[i * 2]) - camX;
			int ay = (int)lroundf(obj->polygon.points[i * 2 + 1]) - camY;
			int bx = (int)lroundf(obj->polygon.points[j * 2]) - camX;
			int by = (int)lroundf(obj->polygon.points[j * 2 + 1]) - camY;
			SDL_RenderDrawLine(renderer, ax, ay, bx, by);
		}
	} else {
		SDL_Rect r = obj->rect;
		r.x -= camX;
		r.y -= camY;
		SDL_RenderDrawRect(renderer, &r);
	}
}

int tilemapInit(Tilemap *tm, const char *filename, SDL_Renderer *renderer){
	tm->map = NULL;
	tm->tileset = NULL;
	tm->tilesetCount = 0;
	clearTileObject(&collisionHitCache);
	
	printf("Attempting to load tilemap: %s\n", filename);
	tm->map = tmx_load(filename);
	if(!tm->map){
		fprintf(stderr, "Failed to load tilemap: %s\n", filename);
		fprintf(stderr, "TMX Error: %s\n", tmx_strerr());
		return -1;
	}
	printf("Loaded tilemap: %s (tiles: %dx%d, tile size: %dx%d)\n", filename, tm->map->width, tm->map->height, tm->map->tile_width, tm->map->tile_height);
	
	// Debug: print layer information
	tmx_layer* layer = tm->map->ly_head;
	int layer_count = 0;
	while(layer){
		printf("Layer %d: %s (visible: %d, type: %d)\n", layer_count, layer->name, layer->visible, layer->type);
		layer_count++;
		layer = layer->next;
	}

	// Load tilesets
	tmx_tileset_list* ts_list = tm->map->ts_head;
	tm->tilesetCount = 0;
	while(ts_list){
		tm->tilesetCount++;
		ts_list = ts_list->next;
	}
	
	if(tm->tilesetCount > 0){
		tm->tileset = malloc((size_t)tm->tilesetCount * sizeof(SDL_Texture*));
		if(!tm->tileset){
			fprintf(stderr, "Failed to allocate tileset texture array\n");
			tmx_map_free(tm->map);
			tm->map = NULL;
			return -1;
		}
		memset(tm->tileset, 0, (size_t)tm->tilesetCount * sizeof(SDL_Texture*));
		ts_list = tm->map->ts_head;
		int i = 0;
		while(ts_list){
			tmx_tileset *tileset = ts_list->tileset;
			printf("Loading tileset: %s (firstgid: %d)\n", tileset->image->source, ts_list->firstgid);
			
			// Try to load the image
			char imagePath[512];
			snprintf(imagePath, sizeof(imagePath), "../assets/tilemap/%s", tileset->image->source);
			
			SDL_Surface* surface = IMG_Load(imagePath);
			if(!surface){
				printf("Failed to load tileset image %s: %s\n", imagePath, IMG_GetError());
				// Use a fallback - create a simple colored surface
				surface = SDL_CreateRGBSurface(0, tileset->image->width, tileset->image->height, 32, 0, 0, 0, 0);
				SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 200, 200, 200));
			}
			
			tm->tileset[i] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
			
			if(!tm->tileset[i]){
				printf("Failed to create texture from tileset: %s\n", SDL_GetError());
			} else {
				printf("Successfully loaded tileset %d\n", i);
			}
			
			ts_list = ts_list->next;
			i++;
		}
	}

	return 0;
}

void tilemapRender(Tilemap *tm, SDL_Renderer *renderer, Camera* cam){
	if(!tm->map) return;

	tmx_layer* layer = tm->map->ly_head;
	while(layer){
			if(layer->visible && layer->type == L_LAYER){
				int startX = 0, startY = 0, endX = tm->map->width - 1, endY = tm->map->height - 1;
				if(cam && cam->optimizeRender){
					startX = cam->x / tm->map->tile_width - 1; if(startX < 0) startX = 0;
					startY = cam->y / tm->map->tile_height - 1; if(startY < 0) startY = 0;
					endX = (cam->x + cam->w) / tm->map->tile_width + 1; if(endX >= tm->map->width) endX = tm->map->width - 1;
					endY = (cam->y + cam->h) / tm->map->tile_height + 1; if(endY >= tm->map->height) endY = tm->map->height - 1;
				}
				for(int y = startY; y <= endY; y++){
					for(int x = startX; x <= endX; x++){
						unsigned gid = layer->content.gids[(y * tm->map->width) + x];
						if(gid != 0){
							SDL_Rect destRect = {
								x * tm->map->tile_width - (cam ? cam->x : 0),
								y * tm->map->tile_height - (cam ? cam->y : 0),
								tm->map->tile_width,
								tm->map->tile_height
							};

						// Find which tileset this gid belongs to
						tmx_tileset_list* ts_list = tm->map->ts_head;
						int tilesetIndex = 0;
						
						while(ts_list){
							if(gid >= ts_list->firstgid && gid < ts_list->firstgid + ts_list->tileset->tilecount){
								break;
							}
							ts_list = ts_list->next;
							tilesetIndex++;
						}
						
						if(ts_list && tilesetIndex < tm->tilesetCount && tm->tileset[tilesetIndex]){
							// Calculate the source rectangle in the tileset
							tmx_tileset *tileset = ts_list->tileset;
							unsigned localId = gid - ts_list->firstgid;
							int tilesPerRow = tileset->image->width / tm->map->tile_width;
							int srcX = (localId % tilesPerRow) * tm->map->tile_width;
							int srcY = (localId / tilesPerRow) * tm->map->tile_height;
							
							SDL_Rect srcRect = {
								srcX, srcY,
								tm->map->tile_width,
								tm->map->tile_height
							};
							
							SDL_RenderCopy(renderer, tm->tileset[tilesetIndex], &srcRect, &destRect);
						} else {
							// Fallback: render colored rectangles for missing tilesets
							if(gid == 1){
								SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown for tile 1
							} else if(gid == 2){
								SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Green for tile 2
							} else if(gid == 3){
								SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for tile 3
							} else {
								SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray for other tiles
							}
							SDL_RenderFillRect(renderer, &destRect);
						}
					}
				}
			}
		}
		layer = layer->next;
	}
}

bool tilemapCheckCollision(Tilemap *tm, SDL_Rect *rect){
	if(!tm || !tm->map || !rect) return false;
	return findCollisionInLayerList(tm, tm->map->ly_head, rect, NULL);
}

bool tilemapCollectCollectibles(Tilemap *tm, SDL_Rect *rect, int *collectedCount){
	if(collectedCount) *collectedCount = 0;
	if(!tm || !tm->map || !rect) return false;

	bool collectedAny = false;
	tmx_layer *layer = tm->map->ly_head;
	while(layer){
		if(!layer->visible){
			layer = layer->next;
			continue;
		}

		if(layer->type == L_LAYER && !layerShouldBeIgnored(layer->name)){
			int tw = (int)tm->map->tile_width;
			int th = (int)tm->map->tile_height;
			int startX = (int)floorf((float)rect->x / (float)tw) - 1;
			int startY = (int)floorf((float)rect->y / (float)th) - 1;
			int endX = (int)floorf((float)(rect->x + rect->w - 1) / (float)tw) + 1;
			int endY = (int)floorf((float)(rect->y + rect->h - 1) / (float)th) + 1;

			if(startX < 0) startX = 0;
			if(startY < 0) startY = 0;
			if(endX >= (int)tm->map->width) endX = (int)tm->map->width - 1;
			if(endY >= (int)tm->map->height) endY = (int)tm->map->height - 1;

			for(int y = startY; y <= endY; y++){
				for(int x = startX; x <= endX; x++){
					unsigned int idx = (unsigned int)(y * tm->map->width + x);
					unsigned gid = layer->content.gids[idx] & TMX_FLIP_BITS_REMOVAL;
					if(gid == 0) continue;

					tmx_tile *tile = (gid < tm->map->tilecount) ? tm->map->tiles[gid] : NULL;
					if(!tileIsCollectible(tile)) continue;

					int tileX = (x * tw) + layer->offsetx;
					int tileY = (y * th) + layer->offsety;
					if(tileIntersectsRect(tile, tileX, tileY, tw, th, rect)){
						layer->content.gids[idx] = 0;
						collectedAny = true;
						if(collectedCount) (*collectedCount)++;
					}
				}
			}
		}

		layer = layer->next;
	}

	return collectedAny;
}

bool tilemapTouchingDoor(Tilemap *tm, SDL_Rect *rect){
	if(!tm || !tm->map || !rect) return false;

	tmx_layer *layer = tm->map->ly_head;
	while(layer){
		if(!layer->visible){
			layer = layer->next;
			continue;
		}

		if(layer->type == L_LAYER && !layerShouldBeIgnored(layer->name)){
			int tw = (int)tm->map->tile_width;
			int th = (int)tm->map->tile_height;
			int startX = (int)floorf((float)rect->x / (float)tw) - 1;
			int startY = (int)floorf((float)rect->y / (float)th) - 1;
			int endX = (int)floorf((float)(rect->x + rect->w - 1) / (float)tw) + 1;
			int endY = (int)floorf((float)(rect->y + rect->h - 1) / (float)th) + 1;

			if(startX < 0) startX = 0;
			if(startY < 0) startY = 0;
			if(endX >= (int)tm->map->width) endX = (int)tm->map->width - 1;
			if(endY >= (int)tm->map->height) endY = (int)tm->map->height - 1;

			for(int y = startY; y <= endY; y++){
				for(int x = startX; x <= endX; x++){
					unsigned int idx = (unsigned int)(y * tm->map->width + x);
					unsigned gid = layer->content.gids[idx] & TMX_FLIP_BITS_REMOVAL;
					if(gid == 0) continue;

					tmx_tile *tile = (gid < tm->map->tilecount) ? tm->map->tiles[gid] : NULL;
					if(!tileIsDoor(tile)) continue;

					int tileX = (x * tw) + layer->offsetx;
					int tileY = (y * th) + layer->offsety;
					if(tileIntersectsRect(tile, tileX, tileY, tw, th, rect)){
						return true;
					}
				}
			}
		}

		layer = layer->next;
	}

	return false;
}

TileObject* tilemapGetCollisionObject(Tilemap* tm, SDL_Rect* rect){
	if(!tm || !tm->map || !rect) return NULL;
	clearTileObject(&collisionHitCache);
	if(findCollisionInLayerList(tm, tm->map->ly_head, rect, &collisionHitCache)){
		return &collisionHitCache;
	}
	return NULL;
}

int tilemapPolygonRectMTV(const TileObject* to, const SDL_Rect* r, float* nx, float* ny, float* overlap){
	if(!to || !r) return 0;

	if(to->hasPolygon && to->polygon.points && to->polygon.pointCount >= 3){
		return polygonRectSAT(to->polygon.points, to->polygon.pointCount, r, nx, ny, overlap) ? 1 : 0;
	}

	if(!rectsIntersect(&to->rect, r)) return 0;
	return rectRectMTV(&to->rect, r, nx, ny, overlap) ? 1 : 0;
}

void tilemapDebugRender(Tilemap *tm, SDL_Renderer *renderer, SDL_Rect* playerRect, Camera* cam){
	if(!tm || !renderer || !playerRect) return;
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	if(tm->map){
		int tw = (int)tm->map->tile_width;
		int th = (int)tm->map->tile_height;
		tmx_layer* layer = tm->map->ly_head;
		while(layer){
			if(!layer->visible){
				layer = layer->next;
				continue;
			}

			if(layer->type == L_GROUP){
				layer = layer->next;
				continue;
			}

			if(layer->type == L_LAYER && !layerShouldBeIgnored(layer->name)){
				int startX = 0, startY = 0, endX = tm->map->width - 1, endY = tm->map->height - 1;
				if(cam && cam->optimizeRender){
					startX = (int)floorf((((float)cam->x) - (float)layer->offsetx) / (float)tw) - 1; if(startX < 0) startX = 0;
					startY = (int)floorf((((float)cam->y) - (float)layer->offsety) / (float)th) - 1; if(startY < 0) startY = 0;
					endX = (int)floorf((((float)(cam->x + cam->w - 1)) - (float)layer->offsetx) / (float)tw) + 1; if(endX >= (int)tm->map->width) endX = (int)tm->map->width - 1;
					endY = (int)floorf((((float)(cam->y + cam->h - 1)) - (float)layer->offsety) / (float)th) + 1; if(endY >= (int)tm->map->height) endY = (int)tm->map->height - 1;
				}
				for(int y = startY; y <= endY; y++){
					for(int x = startX; x <= endX; x++){
						unsigned gid = layer->content.gids[(y * tm->map->width) + x] & TMX_FLIP_BITS_REMOVAL;
						if(gid == 0) continue;

						int tileX = (x * tw) + layer->offsetx;
						int tileY = (y * th) + layer->offsety;
						tmx_tile *tile = (gid < tm->map->tilecount) ? tm->map->tiles[gid] : NULL;
						bool collectible = tileIsCollectible(tile);

						if(tile && tile->collision){
							for(tmx_object *obj = tile->collision; obj; obj = obj->next){
								if(!obj->visible) continue;
								TileObject candidate = {0};
								if(!buildCollisionObject(obj, (float)tileX, (float)tileY, tw, th, &candidate)) continue;
								drawTileObject(renderer, &candidate, cam, collectible ? (SDL_Color){255, 215, 0, 180} : (SDL_Color){0, 255, 0, 150});
								clearTileObject(&candidate);
							}
						} else {
							TileObject tileRect = {0};
							tileRect.rect.x = tileX;
							tileRect.rect.y = tileY;
							tileRect.rect.w = tw;
							tileRect.rect.h = th;
							drawTileObject(renderer, &tileRect, cam, collectible ? (SDL_Color){255, 215, 0, 180} : (SDL_Color){0, 0, 255, 100});
						}
					}
				}
			} else if(layer->type == L_OBJGR && layer->content.objgr && !layerShouldBeIgnored(layer->name)){
				float baseX = (float)layer->offsetx;
				float baseY = (float)layer->offsety;
				for(tmx_object *obj = layer->content.objgr->head; obj; obj = obj->next){
					if(!obj->visible) continue;
					TileObject candidate = {0};
					if(!buildCollisionObject(obj, baseX, baseY, tw, th, &candidate)) continue;
					drawTileObject(renderer, &candidate, cam, (SDL_Color){255, 165, 0, 150});
					clearTileObject(&candidate);
				}
			}

			layer = layer->next;
		}
	}

	/* Draw player rect */
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_Rect adj = {playerRect->x - (cam ? cam->x : 0), playerRect->y - (cam ? cam->y : 0), playerRect->w, playerRect->h};
	SDL_RenderDrawRect(renderer, &adj);
}

void tilemapClean(Tilemap *tm){
	clearTileObject(&collisionHitCache);
	if(tm->tileset){
		for(int i = 0; i < tm->tilesetCount; i++){
			if(tm->tileset[i])
				SDL_DestroyTexture(tm->tileset[i]);
		}
		free(tm->tileset);
		tm->tileset = NULL;
	}
	if(tm->map){
		tmx_map_free(tm->map);
		tm->map = NULL;
	}
}
