#pragma once

#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

class Game : sf::NonCopyable
{
public:
	Game(const std::string& title, unsigned width, unsigned height);

	void run();


private:
	void processEvents();
	void update(sf::Time dt);
	void render();


private:
	sf::RenderWindow mWindow;
	World mWorld;
	std::string mTitle;
	bool mFullScreen;
};
