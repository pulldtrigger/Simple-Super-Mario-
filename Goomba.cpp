#include "Goomba.hpp"
#include "ResourceHolder.hpp"
#include "CommandQueue.hpp"
#include "Player.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
//#define Debug

Goomba::Goomba(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Textures::Enemies), sf::IntRect(0, 16, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mIsDying(false)
	, mIsCrushed(false)
{
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::White);
	mFootShape.setSize({ bounds.width, 2.f });
	mFootShape.setPosition(0.f, bounds.height / 2.f);
	mFootShape.setOutlineThickness(-0.5f);
	bounds = mFootShape.getLocalBounds();
	mFootShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

unsigned int Goomba::getCategory() const
{
	return Category::Goomba;
}

bool Goomba::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Goomba::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Goomba::die()
{
	auto vel = getVelocity();
	vel.y = -180.f; // jump force
	setVelocity(vel);
	setScale(1.f, -1.f);
	mIsDying = true;
	mBehavors = Dying;
}

bool Goomba::isDying() const
{
	return mIsDying;
}

void Goomba::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f);

	if (isDestroyed())
	{
		std::cout << "Goomba died\n";
		mIsMarkedForRemoval = true;
		return;
	}

	accelerate(Gravity);

	switch (mBehavors)
	{
	case Behavors::Air:
	{
		// just fall
	}
	break;
	case Behavors::Ground:
	{
		auto vel = getVelocity();
		if (vel.y > 0.f) vel.y = 0.f;
		setVelocity(vel);
		if (getFootSenseCount() == 0) { mBehavors = Air; };

		updateAnimation(dt);
	}
	break;
	case Behavors::Dying:
		if (mIsCrushed)
		{
			auto vel = getVelocity();
			if (vel.y > 0.f) vel.y = 0.f;
			setVelocity(vel);
			mDyingTimer += dt;
			if (mDyingTimer >= sf::seconds(1))
				destroy();
		}
	break;
	default: break;
	}

	if (!mIsCrushed)
	{
		Entity::updateCurrent(dt, commands);
	}
}

void Goomba::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

sf::FloatRect Goomba::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Goomba::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Goomba::getFootSenseCount() const
{
	return mFootSenseCount;
}

Type Goomba::getType() const
{
	return mType;
}

void Goomba::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (mBehavors)
	{
	case Behavors::Air:
		switch (other->getType())
		{
		case Type::Brick:
		case Type::Solid:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			mBehavors = Ground;
			break;
		case Type::Projectile:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			die();
			break;
		case Type::SmallPlayer:
		case Type::BigPlayer:
			if (other->isDying()) break;

			if (other->getAbilities() & Player::Invincible)
			{
				move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
				die();
			}
			else
			{
				const static sf::IntRect crushed(16 * 2, 16, 16, 16);
				move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);

				if (manifold.y != 0)
				{
					mSprite.setTextureRect(crushed);
					mIsDying = true;
					mIsCrushed = true;
					mBehavors = Dying;
				}
			}

		break;
		default: break;
		}
		break;

	case Behavors::Ground:
		switch (other->getType())
		{
		case Type::Solid:
		case Type::Brick:
		case Type::Goomba:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			if (manifold.x != 0)
			{
				auto vel = getVelocity();
				vel.x = -vel.x;
				setVelocity(vel);
			}
			break;
		case Type::Projectile:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			die();
			break;
		case Type::SmallPlayer:
		case Type::BigPlayer:
			if (other->isDying()) break;

			if (other->getAbilities() & Player::Invincible)
			{
				move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
				die();
			}
			else
			{
				const static sf::IntRect crushed(16 * 2, 16, 16, 16);
				move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);

				if (manifold.y != 0)
				{
					mSprite.setTextureRect(crushed);
					mIsDying = true;
					mIsCrushed = true;
					mBehavors = Dying;
				}
			}

			break;
		default: break;
		}
		break;

	case Behavors::Dying:
		break;
	default: break;
	}
}

void Goomba::updateAnimation(sf::Time dt)
{
	auto textureRect = mSprite.getTextureRect();
	const static auto numFrames = 2u;
	const static auto textureOffest = 0;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 5.f;
	const static auto textureBounds = sf::Vector2i(textureRect.width * numFrames, textureRect.height);
	const static auto startTexture = sf::IntRect(textureOffest, textureRect.top, textureRect.width, textureRect.height);

	if (mElapsedTime <= sf::Time::Zero)
	{
		mElapsedTime += animationInterval / animateRate;

		if (textureRect.left + textureRect.width < textureBounds.x + textureOffest)
			textureRect.left += textureRect.width;
		else
			textureRect = startTexture;
	}
	else
	{
		mElapsedTime -= dt;
	}

	mSprite.setTextureRect(textureRect);
}