#include <stdio.h>
#include <assert.h>
#include "include/physics.h"

int main() {
    printf("Running basic tests for Outta Here game...\n");
    
    // Test physics functions
    float vy = 0.0f;
    physics_apply_gravity(&vy, 1.0f);
    assert(vy == 980.0f);
    printf("✓ Gravity test passed\n");
    
    // Test collision detection
    SDL_Rect rect1 = {10, 10, 20, 20};
    SDL_Rect rect2 = {15, 15, 20, 20};
    SDL_Rect rect3 = {50, 50, 20, 20};
    
    assert(physics_check_rect_collision(&rect1, &rect2) == true);
    assert(physics_check_rect_collision(&rect1, &rect3) == false);
    printf("✓ Collision detection tests passed\n");
    
    // Test rect creation
    SDL_Rect rect = physics_get_rect(100.5f, 200.7f, 32, 32);
    assert(rect.x == 100);
    assert(rect.y == 200);
    assert(rect.w == 32);
    assert(rect.h == 32);
    printf("✓ Rect creation test passed\n");
    
    printf("All basic tests passed! 🎮\n");
    return 0;
}