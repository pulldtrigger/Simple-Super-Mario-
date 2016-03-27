#pragma once


// Entity/scene node category, used to dispatch commands and collisions
namespace Category
{
	enum Type
	{
		None = 0,
		Player = 1 << 0,
		Solid = 1 << 1,
		All = Player | Solid,
	};
}