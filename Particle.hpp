#pragma once


#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>


struct Particle
{
	enum Type
	{
		Splash,
		ParticleCount
	};

	sf::Vector2f position;
	sf::Vector2f velocity;
	float rotation;
	float rotationSpeed;
	sf::Color color;
	sf::Time lifetime;

};