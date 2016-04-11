#include "DataTables.hpp"
#include "Enemy.hpp"
#include "Item.hpp"
#include "Player.hpp"
#include "Tile.hpp"
#include "Projectile.hpp"


std::vector<ItemData> data::initializeItemData()
{
	std::vector<ItemData> data(Item::TypeCount);


	data[Item::StaticCoin].texture = Textures::Items;
	data[Item::StaticCoin].textureRect = sf::IntRect(0, 16 * 6, 16, 16);

	data[Item::MoveableCoin].texture = Textures::Items;
	data[Item::MoveableCoin].textureRect = sf::IntRect(0, 16 * 7, 16, 16);

	data[Item::TransformMushroom].texture = Textures::Items;
	data[Item::TransformMushroom].textureRect = sf::IntRect(0, 0, 16, 16);

	return data;
}

std::vector<PlayerData> data::initializePlayerData()
{

	std::vector<PlayerData> data(Player::TypeCount);

	data[Player::BigPlayer].texture = Textures::Player;
	data[Player::BigPlayer].idleRect = sf::IntRect(80, 0, 16, 32);
	data[Player::BigPlayer].directionRect = sf::IntRect(80 + (16 * 4), 0, 16, 32);
	data[Player::BigPlayer].jumpRect = sf::IntRect(80 + (16 * 5), 0, 16, 32);

	data[Player::BigPlayer].idleFireRect = sf::IntRect(80, 0 + 96, 16, 32);
	data[Player::BigPlayer].directionFireRect = sf::IntRect(80 + (16 * 4), 0 + 96, 16, 32);
	data[Player::BigPlayer].jumpFireRect = sf::IntRect(80 + (16 * 5), 0 + 96, 16, 32);

	data[Player::SmallPlayer].texture = Textures::Player;
	data[Player::SmallPlayer].idleRect = sf::IntRect(80, 32, 16, 16);
	data[Player::SmallPlayer].directionRect = sf::IntRect(80 + (16 * 4), 32, 16, 16);
	data[Player::SmallPlayer].jumpRect = sf::IntRect(80 + (16 * 5), 32, 16, 16);

	data[Player::SmallPlayer].idleFireRect = sf::IntRect(80, 32 + 96, 16, 16);
	data[Player::SmallPlayer].directionFireRect = sf::IntRect(80 + (16 * 4), 32 + 96, 16, 16);
	data[Player::SmallPlayer].jumpFireRect = sf::IntRect(80 + (16 * 5), 32 + 96, 16, 16);

	return data;
}

std::vector<TileData> data::initializeTileData()
{
	std::vector<TileData> data(Tile::TypeCount);

	data[Tile::Brick].texture = Textures::Tile;
	data[Tile::Brick].textureRect = sf::IntRect(16, 0, 16, 16);
	data[Tile::Brick].canAnimate = false;
	data[Tile::Brick].idleRect = sf::IntRect(16 * 27, 0, 16, 16);
	data[Tile::Block] = data[Tile::Brick];

	data[Tile::SoloCoinBox].texture = Textures::Tile;
	data[Tile::SoloCoinBox].textureRect = sf::IntRect(16 * 24, 0, 16, 16);
	data[Tile::SoloCoinBox].canAnimate = true;
	data[Tile::SoloCoinBox].idleRect = sf::IntRect(16 * 27, 0, 16, 16);
	data[Tile::CoinsBox] = data[Tile::TransformBox] = data[Tile::SolidBox] = data[Tile::SoloCoinBox];

	return data;
}