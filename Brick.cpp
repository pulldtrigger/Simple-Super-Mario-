#include "Brick.hpp"
#include "ResourceHolder.hpp"
#include "ParticleNode.hpp"
#include "CommandQueue.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
//#define Debug

Brick::Brick(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Brick), sf::IntRect(17, 0, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mIsHitBySmallPlayer(false)
	, mIsHitByBigPlayer(false)
	, mTimer(sf::Time::Zero)
	, mJump(0.f, -5.2f)
	, mSpawnedExplosion(false)
{
	auto Padding = 2.f;

	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::Cyan);
	mFootShape.setSize({ bounds.width, Padding });
	mFootShape.setPosition(0.f, -bounds.height / 2.f);
	mFootShape.setOutlineThickness(-0.5f);
	bounds = mFootShape.getLocalBounds();
	mFootShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
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
		std::cout << "birck destroyed\n";
		checkExplosion(commands);
		mIsMarkedForRemoval = true;
		return;
	}
	mTimer += dt;
	if (mTimer >= sf::seconds(0.0225f) && mIsHitByBigPlayer)
	{
		move(-mJump);
		destroy();
		mIsHitByBigPlayer = false;
	}

	if (mTimer >= sf::seconds(0.25f) && mIsHitBySmallPlayer)
	{
		move(-mJump);
		mIsHitBySmallPlayer = false;
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
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
		{
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			move(mJump);
		}
		break;
	case Type::BigPlayer:
		if (manifold.y * manifold.z < 0 && !mIsHitByBigPlayer)
		{
			if (getFootSenseCount() != 0u)
			{
				mIsHitByBigPlayer = true;
				mTimer = sf::Time::Zero;
				move(mJump);
			}
			else
			{
				destroy();
			}
		}
		break;
	case Type::Goomba:
		if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
		break;
	case Type::Solid:
	case Type::Brick:
		break;
	default: break;
	}
}

void Brick::checkExplosion(CommandQueue& commands)
{
	if (!mSpawnedExplosion)
	{
		Command explosion;
		explosion.category = Category::ParticleSystem;
		explosion.action = derivedAction<ParticleNode>(std::bind(&ParticleNode::emit, std::placeholders::_1, getWorldPosition()));
		commands.push(explosion);
	}

	mSpawnedExplosion = true;
}