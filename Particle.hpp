#pragma once


#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transformable.hpp>


struct Particle final : public sf::Transformable
{
	enum Type
	{
		Splash,
		ParticleCount
	};

	sf::Vector2f velocity;
	sf::Color color;
	sf::Time lifetime;
};