#include "Player.hpp"
#include "ResourceHolder.hpp"
#include "CommandQueue.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
//#define Debug

Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Textures::Player), (type == Type::BigPlayer) ? sf::IntRect(80, 0, 16, 32) : sf::IntRect(80, 32, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mJumpRect((type == Type::BigPlayer) ? sf::IntRect(80 + (16 * 5), 0, 16, 32) : sf::IntRect(80 + (16 * 5), 32, 16, 16))
	, mIdleRect((type == Type::BigPlayer) ? sf::IntRect(80, 0, 16, 32) : sf::IntRect(80, 32, 16, 16))
	, mIsIdle(true)
	, mFireCommand()
	, mIsFiring(false)
	, mIsFacingLeft(true)
	, mBullets()
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

	mFireCommand.category = Category::SceneMainLayer;
	mFireCommand.action = std::bind(&Player::createProjectile, this, std::placeholders::_1, std::cref(textures));
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

		if (vel.x < 0)
			setScale(-1.f, 1.f);
		else
			setScale(1.f, 1.f);

		mSprite.setTextureRect(mJumpRect);
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

		updateDirection(dt);

		updateAnimation(dt);

	}
	break;
	default:break;
	}

	checkProjectileLaunch(dt, commands);

	Entity::updateCurrent(dt, commands);
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

void Player::applyForce(sf::Vector2f velocity)
{
	mIsIdle = false;
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
				mBehavors = Ground;
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

void Player::updateDirection(sf::Time dt)
{
	auto displacement = static_cast<int>(getVelocity().x * dt.asSeconds());

	if (displacement < 0)
	{
		setScale(-1.f, 1.f);
		mIsIdle = false;
		mIsFacingLeft = false;
	}
	else if (displacement > 0)
	{
		setScale(1.f, 1.f);
		mIsIdle = false;
		mIsFacingLeft = true;
	}
	else if (displacement == 0)
	{
		mIsIdle = true;
		mSprite.setTextureRect(mIdleRect);
	}
}

void Player::updateAnimation(sf::Time dt)
{
	if (mIsIdle) return;

	auto textureRect = mSprite.getTextureRect();
	const static auto numFrames = 3u;
	const static auto textureOffest = textureRect.left + textureRect.width;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 10.f;
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

void Player::fire()
{
	mIsFiring = true;
}

void Player::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	using namespace std::placeholders;
	mBullets.erase(std::remove_if(mBullets.begin(), mBullets.end(), std::mem_fn(&Projectile::isDestroyed)), mBullets.end());
	std::for_each(mBullets.begin(), mBullets.end(), std::bind(&Projectile::adaptProjectileVelocity, _1, getVelocity().x));

	if (!mIsFiring)	return;

	mIsFiring = false;

	commands.push(mFireCommand);
}

void Player::createProjectile(SceneNode& node, const TextureHolder& textures)
{
	auto projectile(std::make_unique<Projectile>(Type::Projectile, textures));

	const static sf::Vector2f offset(mSprite.getGlobalBounds().width / 2.f, 0.f);

	auto sign = (mIsFacingLeft) ? 1.f: -1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	projectile->setVelocity(160.f * sign, -40.f);

	mBullets.emplace_back(projectile.get());

	node.attachChild(std::move(projectile));
}