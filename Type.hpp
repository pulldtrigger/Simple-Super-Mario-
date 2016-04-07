#pragma once


enum class Type
{
	None,
	// players types
	SmallPlayer,
	BigPlayer,

	Block,
	Brick,

	Goomba,
	Projectile,

	// boxes types
	SoloCoinBox,
	CoinsBox,
	TransformBox,
	SolidBox,

	// Item types
	StaticCoin,
	MoveableCoin,
	TransformMushroom,
};
