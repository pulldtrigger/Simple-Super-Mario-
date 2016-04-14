#include "Projectile.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>


Projectile::Projectile(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Items), sf::IntRect(6 * 16, 9 * 16, 8, 8))
	, mIsMarkedForRemoval(false)
	, mTimeDely(sf::Time::Zero)
	, mIsDying(false)
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

	mTimeDely += dt;

	if (mIsDying)
	{
		if (mTimeDely > sf::seconds(0.05f))
			destroy();
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

void Projectile::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getCategory())
	{
	case Category::SolidBox:
	case Category::CoinsBox:
	case Category::SoloCoinBox:
	case Category::TransformBox:
	case Category::Brick:
	case Category::Block:
		if (mIsDying) return;
		if (manifold.x != 0.f)
		{
			mSprite.setTextureRect(sf::IntRect(7 * 16, 9 * 16, 16, 16));
			mIsDying = true;
			mTimeDely = sf::Time::Zero;
		}
		else
		{
			//jump away from floor,
			auto vel = getVelocity();
			vel.y = -250.f; // jump force
			setVelocity(vel);
		}
		break;
	case Category::Goomba:
		if (mIsDying) return;
		mSprite.setTextureRect(sf::IntRect(7 * 16, 9 * 16, 16, 16));
		mTimeDely = sf::Time::Zero;
		mIsDying = true;
		break;
	default: break;
	}
}