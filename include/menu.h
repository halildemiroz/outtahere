#ifndef MENU_H
#define MENU_H

#include <game.h>
#include <tilemap.h>
#include <player.h>

#define MENU_WIDTH 200
#define MENU_HEIGHT 200

#define MENU_START_X (SCREEN_WIDTH / 2) - (MENU_WIDTH / 2)
#define MENU_START_Y (SCREEN_HEIGHT / 2) - (MENU_HEIGHT / 2)

void menuHandleInput(Game* game);

void loadScreenAssets(Game* game);

void startScreen(Game* game);
void gameScreen(Game* game, Tilemap tm, Player player);
void endScreen(Game* game);

#endif
