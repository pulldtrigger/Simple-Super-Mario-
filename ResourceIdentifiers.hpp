#pragma once

// Forward declaration of SFML classes
namespace sf
{
	class Texture;
}

namespace Textures
{
	enum ID
	{
		Player,
		Brick,
		Particle,
		Items,
		Enemies,
	};
}

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier>
class ResourceHolder;

using TextureHolder = ResourceHolder<sf::Texture, Textures::ID>;
