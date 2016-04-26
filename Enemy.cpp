#include "Enemy.hpp"
#include "ResourceHolder.hpp"
#include "CommandQueue.hpp"
#include "DataTables.hpp"
#include "Player.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <functional>
#include<array>

#define Debug

namespace
{
	using namespace std::placeholders;
	const static std::vector<EnemyData>& Table = data::initializeEnemyData();
}

const sf::Vector2f Enemy::Gravity(0.f, 25.f);

Enemy::Enemy(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mIsDying(false)
	, mIsCrushed(false)
	, mUpdateDispatcher()
	, mUpdater()
	, mCollisionDispatcher()
	, mCollision()
{
	switch (mType)
	{
	case Type::Goomba:
	{
		mUpdater = std::bind(&Enemy::behaversUpdate, this, _1);
		mUpdateDispatcher.emplace_back(Behavors::Ground, std::bind(&Enemy::goombaGroundUpdate, this, _1));
		mUpdateDispatcher.emplace_back(Behavors::Dying, std::bind(&Enemy::goombaDyingUpdate, this, _1));
		if (mUpdateDispatcher.capacity() > mUpdateDispatcher.size())
		{
			mUpdateDispatcher.shrink_to_fit();
		}
		mCollision = std::bind(&Enemy::resolveGoomba, this, _1, _2);
		Dispatcher airCollision({
			// Tiles
			{ Category::Brick, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			{ Category::Block, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			{ Category::TransformBox, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			{ Category::CoinsBox, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			{ Category::SoloCoinBox, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			{ Category::SolidBox, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			// Enemies
			{ Category::Goomba, std::bind(&Enemy::airGoombaObjectsCollision, this, _1, _2) },
			// Projectile
			{ Category::Projectile, std::bind(&Enemy::projectileCollision, this, _1, _2) },
			// Player
			{ Category::BigPlayer, std::bind(&Enemy::airGoombaPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Enemy::airGoombaPlayerCollision, this, _1, _2) },
		});

		Dispatcher groundCollision({
			// Tiles
			{ Category::Brick, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			{ Category::Block, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			{ Category::TransformBox, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			{ Category::CoinsBox, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			{ Category::SoloCoinBox, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			{ Category::SolidBox, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			// Enemies
			{ Category::Goomba, std::bind(&Enemy::groundGoombaObjectsCollision, this, _1, _2) },
			// Projectile
			{ Category::Projectile, std::bind(&Enemy::projectileCollision, this, _1, _2) },
			// Player
			{ Category::BigPlayer, std::bind(&Enemy::groundGoombaPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Enemy::groundGoombaPlayerCollision, this, _1, _2) },
		});

		mCollisionDispatcher.emplace_back(Behavors::Air, airCollision);
		mCollisionDispatcher.emplace_back(Behavors::Ground, groundCollision);
		if (mCollisionDispatcher.capacity() > mCollisionDispatcher.size())
		{
			mCollisionDispatcher.shrink_to_fit();
		}
	}
	break;
	case Type::Troopa:

		break;
	case Type::Shell:

	break;
	case Type::Plant:

		break;
	default: break;
	}
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height);

	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::White);
	mFootShape.setSize({ bounds.width, 2.f });

	mFootShape.setPosition(0.f, 0.f);
	mFootShape.setOutlineThickness(-0.5f);
	bounds = mFootShape.getLocalBounds();
	mFootShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

unsigned int Enemy::getCategory() const
{
	const static std::array<unsigned int, Type::TypeCount> category
	{
		Category::Goomba,
		Category::Troopa,
		Category::Shell,
		Category::Plant,
	};

	return category[mType];
}

bool Enemy::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Enemy::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Enemy::die()
{
	auto vel = getVelocity();
	vel.y = -230.f; // jump force
	setVelocity(vel);
	setScale(1.f, -1.f);
	mIsDying = true;
	mBehavors = Dying;
}

bool Enemy::isDying() const
{
	return mIsDying;
}

void Enemy::behaversUpdate(sf::Time dt)
{
	accelerate(Gravity);

	for (const auto& behavor : mUpdateDispatcher)
	{
		if (behavor.first != mBehavors) continue;

		behavor.second(dt);
	}
}

void Enemy::goombaGroundUpdate(sf::Time dt)
{
	auto vel = getVelocity();
	vel.y = std::min({}, vel.y);
	setVelocity(vel);
	if (getFootSenseCount() == 0) { mBehavors = Air; };

	updateAnimation(dt);
}

void Enemy::goombaDyingUpdate(sf::Time dt)
{
	if (!mIsCrushed) return;

	auto vel = getVelocity();
	vel.y = std::min({}, vel.y);
	setVelocity(vel);

	mDyingTimer += dt;

	if (mDyingTimer >= sf::seconds(1))
		destroy();
}

void Enemy::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f);

	if (isDestroyed())
	{
		std::cout << "Enemy died\n";
		mIsMarkedForRemoval = true;
		return;
	}

	if (mUpdater) mUpdater(dt);

	if (mIsCrushed) return;

	Entity::updateCurrent(dt, commands);
}

void Enemy::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

sf::FloatRect Enemy::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Enemy::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Enemy::getFootSenseCount() const
{
	return mFootSenseCount;
}

void Enemy::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	if (mCollision) mCollision(manifold, other);
}

void Enemy::resolveGoomba(const sf::Vector3f& manifold, SceneNode* other)
{
	for (const auto& behavor : mCollisionDispatcher)
	{
		if (behavor.first != mBehavors) continue;

		for (const auto& collider : behavor.second)
		{
			if (collider.first & other->getCategory())
			{
				collider.second(manifold, other);
			}
		}
	}
}

void Enemy::airGoombaPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	const static auto crushed = Table[mType].crushedRect;

	if (other->isDying()) return;
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (other->getAbilities() & Player::Invincible)
	{
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		die();
	}
	else
	{
		if (manifold.y != 0)
		{
			mSprite.setTextureRect(crushed);
			mIsDying = true;
			mIsCrushed = true;
			mBehavors = Dying;
		}
	}
}

void Enemy::groundGoombaPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	const static auto crushed = Table[mType].crushedRect;

	if (other->isDying()) return;
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (other->getAbilities() & Player::Invincible)
	{
		die();
	}
	else
	{
		if (manifold.y != 0)
		{
			mSprite.setTextureRect(crushed);
			mIsDying = true;
			mIsCrushed = true;
			mBehavors = Dying;

			// it works here, at last player pouce if collide with goomba from top
			// NOTE: for some unknown reasons, it doesn't work if i implemented this on player side
			auto vel = other->getVelocity();
			vel.y = -vel.y;//-380.f;
			other->setVelocity(vel);
		}
	}
}

void Enemy::airGoombaObjectsCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	//if (Enemy::Goomba & other->getCategory()) return;

	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	mBehavors = Ground;
}

void Enemy::groundGoombaObjectsCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	//move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (manifold.x != 0)
	{
		auto vel = getVelocity();
		vel.x = -vel.x;
		setVelocity(vel);
	}
}

void Enemy::projectileCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	die();
}

void Enemy::updateAnimation(sf::Time dt)
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