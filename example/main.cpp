#include "GameManager.h"

int main()
{
	GameState* game = CreateGameState("Example2D", 1600, 900);
	GameManager* manager = GM_CreateGameManager(game);

	return 0;
}