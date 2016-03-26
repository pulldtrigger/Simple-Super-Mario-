#pragma once

#include "TileMap.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Sprite.hpp>


namespace sf
{
	class RenderTarget;
}

class World : sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& window);

	void update(sf::Time dt);
	void draw();


private:
	sf::RenderTarget& mWindow;
	sf::View mView;
	TileMap mTileMap;
	// temp vars
	sf::Texture texture;
	sf::Sprite player;
};
