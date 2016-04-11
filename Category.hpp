#pragma once


// Entity/scene node category, used to dispatch commands and collisions
namespace Category
{
	enum Type
	{
		None			= 0,
		BackLayer		= 1 << 0,
		FrontLayer		= 1 << 1,
		// players types
		SmallPlayer = 1 << 2,
		BigPlayer = 1 << 3,

		ParticleSystem = 1 << 4,

		Projectile = 1 << 5,

		// Tile types
		Block = 1 << 6,
		Brick = 1 << 7,
		SoloCoinBox = 1 << 8,
		CoinsBox = 1 << 9,
		TransformBox = 1 << 10,
		SolidBox = 1 << 11,

		// Item types
		StaticCoin = 1 << 12,
		MoveableCoin = 1 << 13,
		TransformMushroom = 1 << 14,

		Goomba = 1 << 15,


		OutOfWorld = SmallPlayer | Projectile | Goomba | BigPlayer | TransformMushroom,
		All = Block | Brick | SoloCoinBox  | CoinsBox | TransformBox | SolidBox | Projectile | StaticCoin | OutOfWorld,
	};
}