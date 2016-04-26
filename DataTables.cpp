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
	data[Item::StaticCoin].textureRect = { 0, 16 * 6, 16, 16 };

	data[Item::MoveableCoin].texture = Textures::Items;
	data[Item::MoveableCoin].textureRect = { 0, 16 * 7, 16, 16 };

	data[Item::Mushroom].texture = Textures::Items;
	data[Item::Mushroom].textureRect = { 0, 0, 16, 16 };

	data[Item::Flower].texture = Textures::Items;
	data[Item::Flower].textureRect = { 0, 16 * 2, 16, 16 };

	data[Item::Star].texture = Textures::Items;
	data[Item::Star].textureRect = { 0, 16 * 3, 16, 16 };

	return data;
}

std::vector<EnemyData> data::initializeEnemyData()
{
	std::vector<EnemyData> data(Enemy::TypeCount);

	data[Enemy::Goomba].texture = Textures::Enemies;
	data[Enemy::Goomba].textureRect = { 0, 16, 16, 16 };
	data[Enemy::Goomba].crushedRect = { 16 * 2, 16, 16, 16 };

	data[Enemy::Troopa].texture = Textures::Enemies;
	data[Enemy::Troopa].textureRect = { 0, 16, 16, 16 };
	data[Enemy::Troopa].crushedRect = { 16 * 2, 16, 16, 16 };

	data[Enemy::Shell].texture = Textures::Enemies;
	data[Enemy::Shell].textureRect = { 0, 16, 16, 16 };
	data[Enemy::Shell].crushedRect = { 16 * 2, 16, 16, 16 };

	data[Enemy::Plant].texture = Textures::Enemies;
	data[Enemy::Plant].textureRect = { 0, 16, 16, 16 };
	data[Enemy::Plant].crushedRect = { 16 * 2, 16, 16, 16 };

	return data;
}

std::vector<PlayerData> data::initializePlayerData()
{
	std::vector<PlayerData> data(Player::TypeCount);

	data[Player::BigPlayer].texture = Textures::Player;
	data[Player::BigPlayer].idleRect = { 80, 0, 16, 32 };
	data[Player::BigPlayer].directionRect = { 80 + (16 * 4), 0, 16, 32 };
	data[Player::BigPlayer].jumpRect = { 80 + (16 * 5), 0, 16, 32 };

	data[Player::BigPlayer].idleFireRect = { 80, 0 + 96, 16, 32 };
	data[Player::BigPlayer].directionFireRect = { 80 + (16 * 4), 0 + 96, 16, 32 };
	data[Player::BigPlayer].jumpFireRect = { 80 + (16 * 5), 0 + 96, 16, 32 };

	data[Player::SmallPlayer].texture = Textures::Player;
	data[Player::SmallPlayer].idleRect = { 80, 32, 16, 16 };
	data[Player::SmallPlayer].directionRect = { 80 + (16 * 4), 32, 16, 16 };
	data[Player::SmallPlayer].jumpRect = { 80 + (16 * 5), 32, 16, 16 };

	data[Player::SmallPlayer].idleFireRect = { 80, 32 + 96, 16, 16 };
	data[Player::SmallPlayer].directionFireRect = { 80 + (16 * 4), 32 + 96, 16, 16 };
	data[Player::SmallPlayer].jumpFireRect = { 80 + (16 * 5), 32 + 96, 16, 16 };

	return data;
}

std::vector<TileData> data::initializeTileData()
{
	std::vector<TileData> data(Tile::TypeCount);

	data[Tile::Brick].texture = Textures::Tile;
	data[Tile::Brick].textureRect = { 16, 0, 16, 16 };
	data[Tile::Brick].canAnimate = false;
	data[Tile::Brick].idleRect = { 16 * 27, 0, 16, 16 };
	data[Tile::Block] = data[Tile::Brick];

	data[Tile::SoloCoinBox].texture = Textures::Tile;
	data[Tile::SoloCoinBox].textureRect = { 16 * 24, 0, 16, 16 };
	data[Tile::SoloCoinBox].canAnimate = true;
	data[Tile::SoloCoinBox].idleRect = { 16 * 27, 0, 16, 16 };
	data[Tile::CoinsBox] = data[Tile::TransformBox] = data[Tile::FireBox] 
						 = data[Tile::ShiftBox] = data[Tile::SolidBox] 
						 = data[Tile::SoloCoinBox];

	return data;
}