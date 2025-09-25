#ifndef PHYSICS_H
#define PHYSICS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define GRAVITY 980.0f

// Physics functions
void physics_apply_gravity(float *vy, float dt);
bool physics_check_rect_collision(SDL_Rect *a, SDL_Rect *b);
SDL_Rect physics_get_rect(float x, float y, int width, int height);

#endif // PHYSICS_H