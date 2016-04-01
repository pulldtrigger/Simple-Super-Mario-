#pragma once


// Entity/scene node category, used to dispatch commands and collisions
namespace Category
{
	enum Type
	{
		None = 0,
		SceneMainLayer = 1 << 0,
		Player = 1 << 1,
		Solid = 1 << 2,
		Brick = 1 << 3,
		Projectile = 1 << 4,
		ParticleSystem = 1 << 5,
		All = Player | Solid | Brick | Projectile,
	};
}