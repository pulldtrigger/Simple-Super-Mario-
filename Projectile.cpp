#include "Projectile.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>


Projectile::Projectile(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Items), sf::IntRect(96, 144, 8, 8))
	, mIsMarkedForRemoval(false)
{
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

unsigned int Projectile::getCategory() const
{
	return Category::Projectile;
}

bool Projectile::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Projectile::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f);

	if (isDestroyed())
	{
		std::cout << "dead\n";
		mIsMarkedForRemoval = true;
		return;
	}

	accelerate(Gravity);

	rotate(10.f);

	Entity::updateCurrent(dt, commands);
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

Type Projectile::getType() const
{
	return mType;
}

void Projectile::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::Brick:
	case Type::Block:
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		if (manifold.x != 0.f)
		{
			destroy();
		}
		else
		{
			//jump away from floor,
			auto vel = getVelocity();
			vel.y = -250.f; // jump force
			setVelocity(vel);
		}
		break;
	case Type::Goomba:
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		destroy();
		break;
	default: break;
	}
}

void Projectile::adaptProjectileVelocity(float vx)
{
	auto vel = getVelocity();
	if (std::fabs(vel.x) > std::fabs(vx) || (vel.x * vx) < 0) return;
	vel.x = vx * 1.5f;
	setVelocity(vel);
}