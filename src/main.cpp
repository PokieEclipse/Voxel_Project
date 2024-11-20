#include <iostream>
#include "Core/game.h"

#define SETUP_ERROR -1

int main(int argc, char** argv)
{
	Game game;
	
	if (game.SetupGame() == SETUP_ERROR)
	{
		std::cout << "Failed in setup game stage! Closing application." << std::endl;
		return -1;
	}

	game.MainLoop();

	return 0;
}