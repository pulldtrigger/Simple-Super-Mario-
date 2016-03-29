#include "Player.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
//#define Debug

Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Textures::Player), sf::IntRect(180 + 29, 0, 16, 16))//sf::IntRect(209 /*+ 29 * 2*/, 52, 16, 32)
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
{
	const auto Padding = 2.f;

	const auto bounds = mSprite.getLocalBounds();
	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::White);
	mFootShape.setSize({ bounds.width, Padding });
	mFootShape.setPosition(0.f, bounds.height);
	mFootShape.setOutlineThickness(-0.5f);
}

unsigned int Player::getCategory() const
{
	return Category::Player;
}

bool Player::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Player::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Player::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f); //25x-270//50X-400

	if(isDestroyed())
	{
		std::cout << "dead\n";
		mIsMarkedForRemoval = true;
		return;
	}

	accelerate(Gravity);

	switch (mBehavors)
	{
	case Behavors::Air:
	{
		auto vel = getVelocity();
		vel.x *= 0.8f;
		setVelocity(vel);
	}
	break;
	case Behavors::Ground:
	{
		auto vel = getVelocity();
		if (vel.y > 0.f) vel.y = 0.f;

		vel.x *= 0.86f;
		setVelocity(vel);

		if (getFootSenseCount() == 0u)
		{
			//nothing underneath so should be falling / jumping
			mBehavors = Air;
		}
	}
	break;
	default:break;
	}
	Entity::updateCurrent(dt, commands);
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
	target.draw(mSprite, states);
}

void Player::applyForce(sf::Vector2f velocity)
{
	accelerate((getFootSenseCount() == 0u) ? sf::Vector2f(velocity.x, 0.f) : velocity);
}

sf::FloatRect Player::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Player::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Player::getFootSenseCount() const
{
	return mFootSenseCount;
}

Type Player::getType() const
{
	return mType;
}

void Player::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (mBehavors)
	{
	case Behavors::Air:
		switch (other->getType())
		{
		case Type::Brick:
			if (manifold.x != 0.f) //if side collision prevents shifting vertically up
			{
				auto vel = getVelocity();
				vel.x = -vel.x;
				setVelocity(vel);
			}
			else
			{
				mBehavors = Ground;
				if (manifold.y * manifold.z > 0) // collide with brick from bottom
				{
					move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
					auto vel = getVelocity();
					vel.y = -vel.y;
					setVelocity(vel);
				}
				else
				{
					move(sf::Vector2f(manifold.x, manifold.y) * -manifold.z);
				}
			}
			break;
		case Type::Solid:
			if (manifold.x != 0.f) //if side collision prevents shifting vertically up
			{
				setVelocity({});
			}
			else
			{
				if (manifold.y * manifold.z > 0)
					move(sf::Vector2f(manifold.x, manifold.y) * -manifold.z);
				else
					move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);

				mBehavors = Ground;
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
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			if (manifold.x != 0)
				setVelocity({}); //we hit a wall so stop
			break;
		default: break;
		}
	}
}
