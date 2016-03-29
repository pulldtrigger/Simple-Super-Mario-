#include "Brick.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
//#define Debug

Brick::Brick(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Brick), sf::IntRect(160, 24, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mIsHit(false)
	, mTimer(sf::Time::Zero)
	, mJump()
{
	const auto Padding = 2.f;

	const auto bounds = mSprite.getLocalBounds();
	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::Cyan);
	mFootShape.setSize({ bounds.width, Padding });
	mFootShape.setPosition(0.f, -Padding);
	mFootShape.setOutlineThickness(-0.5f);
}

unsigned int Brick::getCategory() const
{
	return Category::Brick;
}

bool Brick::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Brick::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Brick::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isDestroyed())
	{
		std::cout << "dead\n";
		mIsMarkedForRemoval = true;
		return;
	}

	mTimer += dt;
	if (mTimer >= sf::seconds(0.25f) && mIsHit)
	{
		mJump.y = -mJump.y;
		move(mJump);
		mIsHit = false;
	}

}

void Brick::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

sf::FloatRect Brick::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Brick::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Brick::getFootSenseCount() const
{
	return mFootSenseCount;
}

Type Brick::getType() const
{
	return mType;
}

void Brick::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (manifold.y * manifold.z < 0 && !mIsHit)
		{
			mIsHit = true;
			mTimer = sf::Time::Zero;
			// mario has small size, so pentration is low. mul by desire ratio
			mJump = sf::Vector2f(manifold.x, manifold.y) * manifold.z * 4.5f;
			move(mJump);

		}
		break;
	case Type::BigPlayer:
		destroy();
		break;
	case Type::Solid:
	case Type::Brick:
	default: break;
	}
}