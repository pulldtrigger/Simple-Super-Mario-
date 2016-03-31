#include "Affectors.hpp"
#include "Particle.hpp"
#include "Utility.hpp"


ForceAffector::ForceAffector(sf::Vector2f force)
	: mForce(force)
{
}

void ForceAffector::operator() (Particle& particle, sf::Time dt)
{
	particle.velocity += mForce * dt.asSeconds();
}

RotateAffector::RotateAffector(float rotation)
	: mRotation(rotation)
{
}

void RotateAffector::operator() (Particle& particle, sf::Time dt)
{
	particle.rotate(mRotation * dt.asSeconds());
}