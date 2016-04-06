#pragma once


// Entity/scene node category, used to dispatch commands and collisions
namespace Category
{
	enum Type
	{
		None			= 0,
		SceneMainLayer	= 1 << 0,
		Player			= 1 << 1,
		Tile			= 1 << 2,
		Projectile		= 1 << 3,
		ParticleSystem	= 1 << 4,
		Goomba			= 1 << 5,

		OutOfWorld = Player | Projectile | Goomba,
		All = Player | Tile | Projectile | Goomba,
	};
}