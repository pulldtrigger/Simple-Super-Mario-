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
		FireBox = 1 << 11,
		ShiftBox = 1 << 12,
		SolidBox = 1 << 13,

		// Item types
		StaticCoin = 1 << 14,
		MoveableCoin = 1 << 15,
		Mushroom = 1 << 16,
		Flower = 1 << 17,
		Star = 1 << 18,


		Goomba = 1 << 19,
		Troopa = 1 << 20,
		Shell = 1 << 21,
		Plant = 1 << 22,

		OutOfWorld = SmallPlayer | BigPlayer | Projectile | Goomba | Troopa | Shell | Mushroom | Star,
		All = Block | Brick | SoloCoinBox  | CoinsBox | TransformBox | FireBox | ShiftBox | SolidBox | Flower | Plant | StaticCoin | OutOfWorld,
	};
}