#include "Game.hpp"

#include <stdexcept>
#include <iostream>


int main()
{
	try
	{
		std::string title = "Mario";

		auto width = 1024u;
		auto height = 512u;

		Game game(title, width, height);

		game.run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}
}