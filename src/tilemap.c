#include <tilemap.h>
#include <polygon_collision.h>
#include <string.h>
#include <ctype.h>

int tilemapInit(Tilemap *tm, const char *filename, SDL_Renderer *renderer){
	tm->map = NULL;
	tm->tileset = NULL;
	tm->tilesetCount = 0;
	
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
		tm->tileset = malloc(tm->tilesetCount * sizeof(SDL_Texture*));
		ts_list = tm->map->ts_head;
		int i = 0;
		while(ts_list){
			tmx_tileset *tileset = ts_list->tileset;
			printf("Loading tileset: %s (firstgid: %d)\n", tileset->image->source, ts_list->firstgid);
			
			// Try to load the image
			char imagePath[512];
			snprintf(imagePath, sizeof(imagePath), "../assets/%s", tileset->image->source);
			
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

static int str_contains_ci(const char* hay, const char* needle){
	if(!hay || !needle) return 0;
	size_t n = strlen(needle);
	for(const char* p = hay; *p; p++){
		size_t i;
		for(i=0; i<n; i++){
			char a = p[i];
			char b = needle[i];
			if(!a) return 0;
			if(a >= 'A' && a <= 'Z') a += 'a'-'A';
			if(b >= 'A' && b <= 'Z') b += 'a'-'A';
			if(a != b) break;
		}
		if(i==n) return 1;
	}
	return 0;
}

static int tilemapLayerIsCollisionLayer(tmx_layer* layer){
	if(!layer) return 0;
	if(!layer->visible) return 0;
	if(layer->type != L_LAYER) return 0;
	/* Skip decorative layer names configured in header */
	{
		const char* ignore_list[] = TILEMAP_IGNORE_LAYER_NAMES;
		size_t ilen = sizeof(ignore_list)/sizeof(ignore_list[0]);
		for(size_t ii = 0; ii < ilen; ++ii){
			if(layer->name && str_contains_ci(layer->name, ignore_list[ii])) return 0;
		}
	}
	return 1;
}

void tilemapRender(Tilemap *tm, SDL_Renderer *renderer){
	if(!tm->map) return;

	tmx_layer* layer = tm->map->ly_head;
	while(layer){
		if(layer->visible && layer->type == L_LAYER){
			for(int y = 0; y < tm->map->height; y++){
				for(int x = 0; x < tm->map->width; x++){
					unsigned gid = layer->content.gids[(y * tm->map->width) + x];
					if(gid != 0){
						SDL_Rect destRect = {
							x * tm->map->tile_width,
							y * tm->map->tile_height,
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
	if(!tm->map) return false;
	
	int sx = rect->x / tm->map->tile_width;
	int sy = rect->y / tm->map->tile_height;

	int ex = (rect->x + rect->w - 1) / tm->map->tile_width;
	int ey = (rect->y + rect->h - 1) / tm->map->tile_height;

	tmx_layer* layer = tm->map->ly_head;
	while(layer){
		if(tilemapLayerIsCollisionLayer(layer)){
			for(int y = sy; y <= ey; y++){
				for(int x = sx; x <= ex; x++){
					if(x >= 0 && x < tm->map->width && y >= 0 && y < tm->map->height){
						unsigned int gid = layer->content.gids[(y * tm->map->width) + x];
						if(gid != 0){
							/* Check for tile with polygon collision (GID 43 = tile ID 42) */
							if(gid == 43){
								/* Hardcoded polygon for tile ID 42: triangle shape */
								float points[] = {0.181818f, 31.9992f, 31.8182f, 32.0909f, 31.9999f, 0.0916883f};
								int pointCount = 3;
								
								float tileX = (float)(x * tm->map->tile_width);
								float tileY = (float)(y * tm->map->tile_height);
								
								if(polygonRectCollision(tileX, tileY, points, pointCount, rect)){
									/* For slopes, be more lenient - only return collision if player is significantly inside */
									float playerCenterX = rect->x + rect->w / 2.0f;
									float playerBottom = rect->y + rect->h;
									float relativeX = playerCenterX - tileX;
									
									/* Calculate expected slope height at player position */
									if(relativeX >= 0 && relativeX <= 32) {
										float slopeHeight = 32.0f - relativeX;
										float slopeTop = tileY + slopeHeight;
										
										/* Only block if player is significantly below the slope surface */
										if(playerBottom > slopeTop + 4) {
											return true;
										}
									} else {
										return true; /* Outside slope area, treat as solid */
									}
								}
							} else {
								/* Standard rectangular collision for other tiles */
								return true;
							}
						}
					}
				}
			}
		}	
		layer = layer->next;
	}
	return false;
}

/* Check if player is standing on top of a slope tile */
bool tilemapIsOnGround(Tilemap *tm, SDL_Rect *rect) {
	if(!tm->map) return false;
	
	/* Check tiles just below the player */
	int sx = rect->x / tm->map->tile_width;
	int sy = (rect->y + rect->h) / tm->map->tile_height; /* Bottom of player */
	int ex = (rect->x + rect->w - 1) / tm->map->tile_width;
	
	tmx_layer* layer = tm->map->ly_head;
	while(layer){
		if(tilemapLayerIsCollisionLayer(layer)){
			for(int x = sx; x <= ex; x++){
				if(x >= 0 && x < tm->map->width && sy >= 0 && sy < tm->map->height){
					unsigned int gid = layer->content.gids[(sy * tm->map->width) + x];
					if(gid != 0){
						if(gid == 43){ /* Slope tile */
							float tileX = (float)(x * tm->map->tile_width);
							float tileY = (float)(sy * tm->map->tile_height);
							
							/* Check if player's bottom edge is touching or just above the slope */
							float playerBottom = rect->y + rect->h;
							float playerCenterX = rect->x + rect->w / 2.0f;
							
							/* For the triangle slope, check if player center X is above the slope line */
							float relativeX = playerCenterX - tileX;
							if(relativeX >= 0 && relativeX <= 32) {
								/* Calculate slope height at player's X position */
								/* Triangle points: (0.18, 31.99), (31.82, 32.09), (32.0, 0.09) */
								/* This is approximately a right triangle from top-right to bottom-left */
								float slopeHeight = 32.0f - relativeX; /* Simple linear slope approximation */
								float slopeTop = tileY + slopeHeight;
								
								/* Player is on ground if their bottom is at or just above slope surface */
								if(playerBottom >= slopeTop - 2 && playerBottom <= slopeTop + 2) {
									return true;
								}
							}
						} else {
							/* Regular tile - check if player bottom is at tile top */
							float tileTop = sy * tm->map->tile_height;
							float playerBottom = rect->y + rect->h;
							if(playerBottom >= tileTop - 2 && playerBottom <= tileTop + 2) {
								return true;
							}
						}
					}
				}
			}
		}
		layer = layer->next;
	}
	return false;
}

/* Stub functions needed by player code */
TileObject* tilemapGetCollisionObject(Tilemap* tm, SDL_Rect* rect){
	/* For now, return NULL - this would return specific collision objects */
	return NULL;
}

int tilemapPolygonRectMTV(const TileObject* to, const SDL_Rect* r, float* nx, float* ny, float* overlap){
	/* Minimal Translation Vector - simplified for now */
	if(nx) *nx = 1.0f;
	if(ny) *ny = 0.0f;  
	if(overlap) *overlap = 1.0f;
	return 1;
}

void tilemapDebugRender(Tilemap *tm, SDL_Renderer *renderer, SDL_Rect* playerRect){
	if(!tm || !renderer || !playerRect) return;
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	/* Draw collision shapes for special tiles */
	if(tm->map){
		int tw = tm->map->tile_width;
		int th = tm->map->tile_height;
		tmx_layer* layer = tm->map->ly_head;
		while(layer){
			if(tilemapLayerIsCollisionLayer(layer)){
				for(int y = 0; y < tm->map->height; y++){
					for(int x = 0; x < tm->map->width; x++){
						unsigned gid = layer->content.gids[(y * tm->map->width) + x];
						
						/* Draw polygon for tile GID 43 (tile ID 42) */
						if(gid == 43){
							SDL_SetRenderDrawColor(renderer, 0, 255, 0, 150);
							int tileX = x * tw;
							int tileY = y * th;
							
							/* Draw the triangle polygon */
							float points[] = {0.181818f, 31.9992f, 31.8182f, 32.0909f, 31.9999f, 0.0916883f};
							int pointCount = 3;
							
							for(int j = 0; j < pointCount; j++){
								int k = (j + 1) % pointCount;
								int ax = tileX + (int)points[j*2];
								int ay = tileY + (int)points[j*2+1];
								int bx = tileX + (int)points[k*2];
								int by = tileY + (int)points[k*2+1];
								SDL_RenderDrawLine(renderer, ax, ay, bx, by);
							}
						} else if(gid != 0) {
							/* Draw regular tile collision box */
							SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
							SDL_Rect tileRect = {x * tw, y * th, tw, th};
							SDL_RenderDrawRect(renderer, &tileRect);
						}
					}
				}
			}
			layer = layer->next;
		}
	}

	/* Draw player rect */
	SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
	SDL_RenderDrawRect(renderer, playerRect);
}

void tilemapClean(Tilemap *tm){
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

