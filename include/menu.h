#ifndef MENU_H
#define MENU_H

#include <game.h>
#include <tilemap.h>
#include <player.h>

void loadScreenAssets(Game* game);

void startScreen(Game* game);
void gameScreen(Game* game, Tilemap tm, Player player);
void endScreen(Game* game);

#endif
