#include "Player.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Textures::Player), sf::IntRect(180, 0, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
{
	const float Padding = 2.f;

	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	setScale(1.5f, 1.5f);

	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::White);
	mFootShape.setSize({ bounds.width, Padding });
	mFootShape.setPosition(0.f, bounds.height / 2.f);
	mFootShape.setOutlineThickness(-1.f);
	bounds = mFootShape.getLocalBounds();
	mFootShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
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
	static const sf::Vector2f Gravity(0.f, 80.f);

	if(isDestroyed())
	{
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
	//target.draw(mFootShape, states);
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
		case Type::FixedSolid:
		case Type::MoveableSolid:
			if (manifold.x != 0.f) //if side collision prevents shifting vertically
				setVelocity({});
			else
			{
				move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
				setVelocity({ getVelocity().x, 0.f });
				mBehavors = Ground;
			}
			break;
		default: break;
		}
		break;

	case Behavors::Ground:
		switch (other->getType())
		{
		case Type::FixedSolid:
		case Type::MoveableSolid:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			if (manifold.x == 0)
				setVelocity({ getVelocity().x, 0.f }); //carry on moving if we hit ground
			else
				setVelocity({}); //we hit a wall so stop
			break;
		default: break;
		}
	}
}