#pragma once


#include "ResourceIdentifiers.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <vector>

struct ItemData
{
	Textures::ID		texture;
	sf::IntRect			textureRect;
};

struct EnemyData
{
	Textures::ID		texture;
	sf::IntRect			textureRect;
	sf::IntRect			crushedRect;
	int					offset;
};

struct PlayerData
{
	Textures::ID		texture;
	sf::IntRect			idleRect;
	sf::IntRect			directionRect;
	sf::IntRect			jumpRect;
	sf::IntRect			idleFireRect;
	sf::IntRect			directionFireRect;
	sf::IntRect			jumpFireRect;
};

struct TileData
{
	Textures::ID		texture;
	sf::IntRect			textureRect;
	sf::IntRect			idleRect;
	bool				canAnimate;
};

namespace data
{
	std::vector<ItemData>		initializeItemData();
	std::vector<EnemyData>		initializeEnemyData();
	std::vector<PlayerData>		initializePlayerData();
	std::vector<TileData>		initializeTileData();
}