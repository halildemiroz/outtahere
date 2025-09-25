#include "physics.h"

void physics_apply_gravity(float *vy, float dt) {
    *vy += GRAVITY * dt;
}

bool physics_check_rect_collision(SDL_Rect *a, SDL_Rect *b) {
    return SDL_HasIntersection(a, b);
}

SDL_Rect physics_get_rect(float x, float y, int width, int height) {
    SDL_Rect rect;
    rect.x = (int)x;
    rect.y = (int)y;
    rect.w = width;
    rect.h = height;
    return rect;
}