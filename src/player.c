#include "player.h"
#include "physics.h"
#include <stdio.h>

int player_init(Player *player, SDL_Renderer *renderer) {
    player->x = 100.0f;
    player->y = 400.0f;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->width = PLAYER_WIDTH;
    player->height = PLAYER_HEIGHT;
    player->on_ground = false;
    player->texture = NULL;
    
    // For now, we'll create a simple colored rectangle as player sprite
    // In a real game, you'd load a texture file here
    // player->texture = IMG_LoadTexture(renderer, "assets/textures/player.png");
    
    printf("Player initialized at position (%.2f, %.2f)\n", player->x, player->y);
    return 0;
}

void player_cleanup(Player *player) {
    if (player->texture) {
        SDL_DestroyTexture(player->texture);
        player->texture = NULL;
    }
}

void player_handle_input(Player *player, Game *game) {
    // Reset horizontal velocity
    player->vx = 0.0f;
    
    // Handle left/right movement
    if (game->keys[SDL_SCANCODE_LEFT] || game->keys[SDL_SCANCODE_A]) {
        player->vx = -PLAYER_SPEED;
    }
    if (game->keys[SDL_SCANCODE_RIGHT] || game->keys[SDL_SCANCODE_D]) {
        player->vx = PLAYER_SPEED;
    }
    
    // Handle jumping
    if ((game->keys[SDL_SCANCODE_SPACE] || game->keys[SDL_SCANCODE_UP] || game->keys[SDL_SCANCODE_W]) && player->on_ground) {
        player->vy = JUMP_SPEED;
        player->on_ground = false;
    }
    
    // Handle quit
    if (game->keys[SDL_SCANCODE_ESCAPE]) {
        game->running = false;
    }
}

void player_update(Player *player, Game *game, float dt) {
    // Apply gravity
    physics_apply_gravity(&player->vy, dt);
    
    // Update position
    player->x += player->vx * dt;
    player->y += player->vy * dt;
    
    // Simple ground collision (temporary until we have proper tilemap collision)
    float ground_level = SCREEN_HEIGHT - 64; // Leave some space for ground
    if (player->y + player->height >= ground_level) {
        player->y = ground_level - player->height;
        player->vy = 0.0f;
        player->on_ground = true;
    } else {
        player->on_ground = false;
    }
    
    // Keep player within screen bounds horizontally
    if (player->x < 0) {
        player->x = 0;
    } else if (player->x + player->width > SCREEN_WIDTH) {
        player->x = SCREEN_WIDTH - player->width;
    }
    
    // Prevent falling through bottom of screen
    if (player->y > SCREEN_HEIGHT) {
        player->y = ground_level - player->height;
        player->vy = 0.0f;
        player->on_ground = true;
    }
}

void player_render(Player *player, SDL_Renderer *renderer) {
    SDL_Rect dest_rect = physics_get_rect(player->x, player->y, player->width, player->height);
    
    if (player->texture) {
        SDL_RenderCopy(renderer, player->texture, NULL, &dest_rect);
    } else {
        // Render as a simple red rectangle for now
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &dest_rect);
    }
}