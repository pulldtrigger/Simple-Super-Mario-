#include "Entity.hpp"
#include "Utility.hpp"

Entity::Entity(int hitpoints)
	: mVelocity()
	, mHitpoints(hitpoints)
{
}

void Entity::setVelocity(sf::Vector2f velocity)
{
	mVelocity = velocity;
}

void Entity::setVelocity(float vx, float vy)
{
	mVelocity.x = vx;
	mVelocity.y = vy;
}

void Entity::destroy()
{
	mHitpoints = 0;
}

void Entity::remove()
{
	destroy();
}

bool Entity::isDestroyed() const
{
	return mHitpoints <= 0;
}

void Entity::updateCurrent(sf::Time dt, CommandQueue&)
{
	auto speed = utility::length(mVelocity) * dt.asSeconds();
	auto velocity = utility::normalise(mVelocity) * speed;

	move(velocity);
}

void Entity::accelerate(sf::Vector2f velocity)
{
	mVelocity += velocity;
}

sf::Vector2f Entity::getVelocity() const
{
	return mVelocity;
}