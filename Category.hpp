#pragma once


// Entity/scene node category, used to dispatch commands and collisions
namespace Category
{
	enum Type
	{
		None			= 0,
		BackLayer		= 1 << 0,
		FrontLayer		= 1 << 1,
		Player			= 1 << 2,
		Tile			= 1 << 3,
		Projectile		= 1 << 4,
		ParticleSystem	= 1 << 5,
		Goomba			= 1 << 6,
		Item			= 1 << 7,

		OutOfWorld = Player | Projectile | Goomba | Item,
		All = Player | Tile | Projectile | Goomba | Item,
	};
}