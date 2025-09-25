#include <game.h>

int main(int argc, char* argv[]){
	Game game = {0};
	
	if(gameInit(&game, "Outta Here") != 0){
		printf("Failed to initialize the game\n");
		return 1;
	}

	gameRun(&game);
	gameClean(&game);

	return 0;
}
