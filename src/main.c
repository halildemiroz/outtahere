#include <stdio.h>
#include <stdlib.h>
#include "game.h"

int main(int argc, char *argv[]) {
    Game game = {0};
    
    printf("Starting Outta Here - Mario-style game\n");
    
    if (game_init(&game) != 0) {
        fprintf(stderr, "Failed to initialize game\n");
        return 1;
    }
    
    game_run(&game);
    game_cleanup(&game);
    
    printf("Game shutdown complete\n");
    return 0;
}