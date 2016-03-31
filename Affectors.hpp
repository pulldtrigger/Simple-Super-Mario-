#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>


struct Particle;

struct ForceAffector
{
	explicit ForceAffector(sf::Vector2f force);

	void operator()(Particle& particle, sf::Time dt);


private:
	sf::Vector2f mForce;
};

struct RotateAffector
{
	explicit RotateAffector(float rotation);

	void operator() (Particle& particle, sf::Time dt);


private:
	float mRotation;
};