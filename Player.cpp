#include "Player.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "CommandQueue.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <functional>

#define Debug

namespace
{
	using namespace std::placeholders;

	bool justOnce = true;
	const static std::vector<PlayerData>& Table = data::initializePlayerData();
}

Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Table[type].texture), Table[type].idleRect)
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mJumpRect(Table[type].jumpRect)
	, mDirectionRect(Table[type].directionRect)
	, mIdleRect(Table[type].idleRect)
	, mDirectionTime(sf::Time::Zero)
	, mCurrentDirection(Right | Up)
	, mPreviousDirection(Right | Up)
	, isChangingDirection(false)
	, isRightFace(true)
	, mAbilities(Regular)
	, mAbilitiesTime(sf::Time::Zero)
	, mFireCommand()
	, mIsFiring(false)
	, mBullets()
	, mTimer(sf::Time::Zero)
	, mAffects(Blinking)
	, mScaleToggle(true)
	, mIsDying(false)
	, mIsSmallPlayerTransformed(false)
	, mCollisionDispatcher()
	, mUpdateDispatcher()
{
	setup();

	mFireCommand.category = Category::BackLayer;
	mFireCommand.action = std::bind(&Player::createProjectile, this, _1, std::cref(textures));

	initialDispatching();
}

void Player::setup()
{
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height);

	mFootShape.setFillColor(sf::Color::Transparent);
	mFootShape.setOutlineColor(sf::Color::White);
	mFootShape.setSize({ bounds.width, 2.f });
	mFootShape.setPosition(0.f, 0.f);
	mFootShape.setOutlineThickness(-0.5f);
	auto footBounds = mFootShape.getLocalBounds();
	mFootShape.setOrigin(footBounds.width / 2.f, footBounds.height / 2.f);
}

void Player::initialDispatching()
{
	Dispatcher airCollision({
		// Tiles
		{ Category::Brick, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::Block, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::TransformBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::FireBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::ShiftBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::CoinsBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::SoloCoinBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		{ Category::SolidBox, std::bind(&Player::airTileCollision, this, _1, _2) },
		// Enemies
		{ Category::Goomba, std::bind(&Player::airEnemyCollision, this, _1, _2) },
		// Items
		{ Category::Mushroom, std::bind(&Player::mushroomCollision, this, _1, _2) },
		{ Category::Flower, std::bind(&Player::flowerCollision, this, _1, _2) },
		{ Category::Star, std::bind(&Player::starCollision, this, _1, _2) },
	});

	Dispatcher groundCollision({
		// Tiles
		{ Category::Brick, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::Block, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::TransformBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::FireBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::ShiftBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::CoinsBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::SoloCoinBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		{ Category::SolidBox, std::bind(&Player::groundTileCollision, this, _1, _2) },
		// Enemies
		{ Category::Goomba, std::bind(&Player::groundEnemyCollision, this, _1, _2) },
		// Items
		{ Category::Mushroom, std::bind(&Player::mushroomCollision, this, _1, _2) },
		{ Category::Flower, std::bind(&Player::flowerCollision, this, _1, _2) },
		{ Category::Star, std::bind(&Player::starCollision, this, _1, _2) },
	});

	mCollisionDispatcher.emplace_back(Behavors::Air, airCollision);
	mCollisionDispatcher.emplace_back(Behavors::Ground, groundCollision);
	if (mCollisionDispatcher.capacity() > mCollisionDispatcher.size())
	{
		mCollisionDispatcher.shrink_to_fit();
	}
	mUpdateDispatcher.emplace_back(Behavors::Air, std::bind(&Player::airUpdate, this, _1));
	mUpdateDispatcher.emplace_back(Behavors::Ground, std::bind(&Player::groundUpdate, this, _1));
	mUpdateDispatcher.emplace_back(Behavors::Dying, std::bind(&Player::dyingUpdate, this, _1));
	if (mUpdateDispatcher.capacity() > mUpdateDispatcher.size())
	{
		mUpdateDispatcher.shrink_to_fit();
	}
}

void Player::airTileCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (manifold.x != 0.f) //if side collision prevents shifting vertically up
	{
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		setVelocity({});
	}
	else
	{
		if (manifold.y * manifold.z > 0)// collide with brick from bottom
		{
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			auto vel = getVelocity();
			vel.y = -vel.y;
			setVelocity(vel);
		}
		//else //NOTE: it makes player bounces rapidly under Boxes
		//{
		mBehavors = Ground;
		mCurrentDirection &= ~(Up);
		mCurrentDirection |= Idle;
		//}
	}
}

void Player::airEnemyCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;
	if (mIsSmallPlayerTransformed) return;
	if (mAbilities & Abilities::Invincible) return;

	auto sideCollisionBigPlayer = [this](const sf::Vector3f& manifold, SceneNode* other)
	{
		mAffects = Death | Pause | Scaling;
		mIsSmallPlayerTransformed = true;
		mAbilities = Regular;
	};

	auto sideCollisionSmallPlayer = [this](const sf::Vector3f& manifold, SceneNode* other)
	{
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		auto vel = getVelocity();
		vel.y = -8.f;//-475.f; // jump force
		vel.x = 0.f;
		setVelocity(vel);
		mSprite.setTextureRect(sf::IntRect(80 + (16 * 6), 32, 16, 16));
		mBehavors = Dying;
		mAffects = Pause;
		mIsDying = true;
	};

	const static std::array<Function, Type::TypeCount> sideCollision
	{
		sideCollisionSmallPlayer,
		sideCollisionBigPlayer
	};

	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (manifold.x != 0) // side collision
	{
		sideCollision[mType](manifold, other);
	}
	// NOTE: for some reasons this doesn't work, better to implement it on goomba side
	//else //if (manifold.y != 0)
	//{
	//	if (manifold.y * manifold.z < 0)
	//	{
	//		auto vel = getVelocity();
	//		vel.y = -vel.y;//-380.f;
	//		setVelocity(vel);
	//	}
	//}
}

void Player::mushroomCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (mType == Type::SmallPlayer)
		applyTransformation();
}

void Player::flowerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (!justOnce) return;
	applyFireable();
	justOnce = false;
}

void Player::starCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	applyInvincible();
}

void Player::groundTileCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (manifold.x != 0)
		setVelocity({}); //we hit a wall so stop
}

void Player::groundEnemyCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;
	if (mIsSmallPlayerTransformed) return;
	if (mAbilities & Abilities::Invincible) return;

	auto sideCollisionBigPlayer = [this](const sf::Vector3f& manifold, SceneNode* other)
	{
		mAffects = Death | Pause | Scaling | Blinking;
		mIsSmallPlayerTransformed = true;
		mAbilities = Regular;
	};

	auto sideCollisionSmallPlayer = [this](const sf::Vector3f& manifold, SceneNode* other)
	{
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		auto vel = getVelocity();
		vel.y = -237.f;//-475.f; // jump force
		vel.x = 0.f;
		setVelocity(vel);
		mSprite.setTextureRect(sf::IntRect(80 + (16 * 6), 32, 16, 16));
		mBehavors = Dying;
		mAffects = Pause;
		mIsDying = true;
	};

	const static std::array<Function, Type::TypeCount> sideCollision
	{
		sideCollisionSmallPlayer,
		sideCollisionBigPlayer
	};

	if (manifold.x != 0)
	{
		sideCollision[mType](manifold, other);
	}
}

unsigned int Player::getCategory() const
{
	const static std::array<unsigned int, Type::TypeCount> category
	{
		Category::SmallPlayer,
		Category::BigPlayer
	};

	return category[mType];
}

bool Player::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

bool Player::isDying() const
{
	return mIsSmallPlayerTransformed || mIsDying;
}

bool Player::paused()
{
	return (mAffects & Pause) == Pause;
}

void Player::applyFireable()
{
	mAffects = Shifting;
	mAbilities |= Fireable;
}

void Player::applyInvincible()
{
	mAffects = Power | Shifting;
	mAbilities |= Invincible;
}

void Player::applyTransformation(Type type)
{
	mType = type;
	mJumpRect = (mAbilities & Fireable) ? Table[mType].jumpFireRect : Table[mType].jumpRect;
	mDirectionRect = (mAbilities & Fireable) ? Table[mType].directionFireRect : Table[mType].directionRect;
	mIdleRect = (mAbilities & Fireable) ? Table[mType].idleFireRect : Table[mType].idleRect;

	mSprite.setTextureRect(mIdleRect); // ugly

	setup();

	if (!(mAffects & (Scaling | Death)))
		mAffects = Scaling;
}

bool Player::scalingEffect(sf::Time dt, sf::Vector2f targetScale)
{
	auto resultScale = mSprite.getScale();

	auto speed = 0.f;
	if (mAffects & Death)
		speed = 48.f;
	else
		speed = 32.f;

	resultScale += (targetScale - resultScale) * speed * dt.asSeconds();

	mSprite.setScale(resultScale);

	return utility::closeEnough(targetScale.y, resultScale.y, 0.0001f);
}

void Player::applyBigPlayerShifting(sf::Time dt)
{
	auto textureRect = mSprite.getTextureRect();

	const static auto numFrames = 10u;
	const static auto textureOffest = 16;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 15.f;
	const static auto textureBounds = sf::Vector2i(textureRect.width, (textureRect.height + textureOffest) * numFrames);
	const static auto startTexture = sf::IntRect(textureRect.left, 0, textureRect.width, textureRect.height);

	if (mAbilitiesTime <= sf::Time::Zero)
	{
		mAbilitiesTime += animationInterval / animateRate;

		if (textureRect.top + textureRect.height < textureBounds.y)
			textureRect.top += textureRect.height + textureOffest;
		else
			textureRect = startTexture;
	}
	else 
	{
		mAbilitiesTime -= dt;
	}

	mSprite.setTextureRect(textureRect);
}

void Player::applySmallPlayerShifting(sf::Time dt)
{
	auto textureRect = mSprite.getTextureRect();

	const static auto numFrames = 10u;
	const static auto textureOffest = 32;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 15.f;
	const static auto textureBounds = sf::Vector2i(textureRect.width, (textureRect.height + textureOffest) * numFrames);
	const static auto startTexture = sf::IntRect(textureRect.left, textureOffest, textureRect.width, textureRect.height);

	if (mAbilitiesTime <= sf::Time::Zero)
	{
		mAbilitiesTime += animationInterval / animateRate;

		if (textureRect.top + textureRect.height < textureBounds.y)
			textureRect.top += textureRect.height + textureOffest;
		else
			textureRect = startTexture;
	}
	else
	{
		mAbilitiesTime -= dt;
	}

	mSprite.setTextureRect(textureRect);
}

void Player::playEffects(sf::Time dt)
{

	if (mAffects & Nothing) return;

	if (mAffects & Scaling)
	{
		if (mScaleToggle)
		{
			if (scalingEffect(dt, { 1.f, 0.5f }))
				mScaleToggle = !mScaleToggle;
		}
		else
		{
			if (scalingEffect(dt, { 1.f, 1.f }))
				mScaleToggle = !mScaleToggle;
		}
	}

	if (mAffects & Blinking)
	{
		mSprite.setColor({ 255u, 255u, 255u, static_cast<sf::Uint8>(utility::random(1, 255)) });
	}

	if (mAffects & Shifting)
	{
		if (mType == Type::BigPlayer)
		{
			applyBigPlayerShifting(dt);
		}
		else
		{
			applySmallPlayerShifting(dt);
		}
	}

	mTimer += dt;

	if (mAffects & Death)
	{
		if (mTimer <= sf::seconds(1.5f)) return;
		mAbilities = Regular;
		applyTransformation(Type::SmallPlayer);
		mAffects = Blinking;
		mTimer = sf::Time::Zero;
		return;
	}

	if (mAffects & Power)
	{
		if (mTimer <= sf::seconds(5.5f)) return;
		mAbilities &= ~(Invincible);
	}

	if (mTimer <= sf::seconds(1.f)) return;

	if (mAffects & Scaling)
		mSprite.setScale({ 1.f,  1.f });

	if (mAffects & Blinking)
		mSprite.setColor(sf::Color::White);

	if (mAffects & Shifting)
	{
		mJumpRect = (mAbilities & Fireable) ? Table[mType].jumpFireRect : Table[mType].jumpRect;
		mDirectionRect = (mAbilities & Fireable) ? Table[mType].directionFireRect : Table[mType].directionRect;
		mIdleRect = (mAbilities & Fireable) ? Table[mType].idleFireRect : Table[mType].idleRect;
		mSprite.setTextureRect(mIdleRect);
	}

	mScaleToggle = true;
	mAffects = Nothing;
	if (mIsSmallPlayerTransformed) mIsSmallPlayerTransformed = false;
	mTimer = sf::Time::Zero;
}

sf::FloatRect Player::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Player::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f);

	if(isDestroyed())
	{
		std::cout << "Mario died\n";
		mIsMarkedForRemoval = true;
		return;
	}

	accelerate(Gravity);

	for (const auto& behavor : mUpdateDispatcher)
	{
		if (behavor.first != mBehavors) continue;

		 behavor.second(dt);
	}

	playEffects(dt);

	if (mAffects & Death) return;

	updateDirection(dt);

	updateAnimation(dt);

	checkProjectiles();

	checkProjectileLaunch(dt, commands);

	Entity::updateCurrent(dt, commands);

	debug << "X: " << getWorldPosition().x << "\n"
		  << "Y: " << getWorldPosition().y;
}

void Player::airUpdate(sf::Time dt)
{
	auto vel = getVelocity();
	vel.x *= 0.7f;//0.6f;//0.8f;
	if (mIsSmallPlayerTransformed) vel.y = 0.f;
	setVelocity(vel);

	if (vel.x > 0)
	{
		mCurrentDirection &= ~(Left);
		mCurrentDirection |= Right;
	}
	else if (vel.x < 0)
	{
		mCurrentDirection &= ~(Right);
		mCurrentDirection |= Left;
	}
}

void Player::groundUpdate(sf::Time dt)
{
	auto vel = getVelocity();
	vel.y = std::min({}, vel.y);
	vel.x *= 0.8f;
	setVelocity(vel);

	auto displacement = static_cast<int>(vel.x * dt.asSeconds());

	if (displacement < 0)
	{
		mCurrentDirection &= ~(Idle | Right);
		mCurrentDirection |= Left;
	}
	else if (displacement > 0)
	{
		mCurrentDirection &= ~(Idle | Left);
		mCurrentDirection |= Right;
	}
	else
	{
		mCurrentDirection &= ~(Right | Left | Up);
		mCurrentDirection |= Idle;
	}

	if (mFootSenseCount == 0u)
	{
		//nothing underneath so should be falling / jumping
		mBehavors = Air;
		mCurrentDirection &= ~(Idle);
		mCurrentDirection |= Up;
	}
}

void Player::dyingUpdate(sf::Time dt)
{
	auto vel = getVelocity();
	vel.x = 0.f;
	setVelocity(vel);
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

void Player::resolve(const sf::Vector3f& manifold, SceneNode* other)
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

void Player::updateDirection(sf::Time dt)
{
	if (!(mPreviousDirection ^ mCurrentDirection)) return;

	if (mCurrentDirection & Right)
	{
		setScale(1.f, 1.f);
		isRightFace = true;
		if (mFootSenseCount != 0u)
		{
			isChangingDirection = true;
			mSprite.setTextureRect(mDirectionRect);
		}
	}

	if (mCurrentDirection & Left)
	{
		setScale(-1.f, 1.f);
		isRightFace = false;
		if (mFootSenseCount != 0u)
		{
			isChangingDirection = true;
			mSprite.setTextureRect(mDirectionRect);
		}
	}

	if (mCurrentDirection & Up)
	{
		mSprite.setTextureRect(mJumpRect);
	}

	if (mCurrentDirection & Idle)
	{
		//isDirectionAnimation = true;
		mSprite.setTextureRect(mIdleRect);
	}

	mPreviousDirection = mCurrentDirection;
}

void Player::updateAnimation(sf::Time dt)
{
	if (mIsDying) return;
	if (mCurrentDirection & (Idle | Up)) return;

	// changing direction anim
	if (isChangingDirection && mDirectionTime <= sf::Time::Zero)
	{
		isChangingDirection = false;
		mDirectionTime += sf::seconds(0.15f);
		mSprite.setTextureRect(mIdleRect);
	}
	else if (isChangingDirection)
	{
		mDirectionTime -= dt;
	}

	auto textureRect = mSprite.getTextureRect();
	auto animateRate = (mAbilities & Invincible) ? 25.f : 15.f;
	const static auto numFrames = 3u;
	const static auto textureOffest = textureRect.left + textureRect.width;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto textureBounds = sf::Vector2i(textureRect.width * numFrames, textureRect.height);
	const auto startTexture = sf::IntRect(textureOffest, textureRect.top, textureRect.width, textureRect.height);

	// running anim
	if (!isChangingDirection && mElapsedTime <= sf::Time::Zero)
	{
		mElapsedTime += animationInterval / animateRate;

		if (textureRect.left + textureRect.width < textureBounds.x + textureOffest)
			textureRect.left += textureRect.width;
		else
			textureRect = startTexture;
	}
	else if (!isChangingDirection)
	{
		mElapsedTime -= dt;
	}

	mSprite.setTextureRect(textureRect);
}

unsigned int Player::getAbilities() const
{
	return mAbilities;
}

void Player::fire()
{
	mIsFiring = true;
}

void Player::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	if (!mIsFiring || !(mAbilities & Abilities::Fireable))	return;

	mIsFiring = false;

	commands.push(mFireCommand);
}

void Player::checkProjectiles()
{
	mBullets.erase(std::remove_if(mBullets.begin(), mBullets.end(), std::mem_fn(&Projectile::isDestroyed)), mBullets.end());
}

void Player::createProjectile(SceneNode& node, const TextureHolder& textures)
{
	auto projectile(std::make_unique<Projectile>(Projectile::PlayerProjectile, textures));

	const sf::Vector2f offset(mSprite.getLocalBounds().width / 2.f, -mSprite.getLocalBounds().height / 2.f);

	auto sign = (isRightFace) ? 1.f: -1.f;
	projectile->setPosition(getWorldPosition() + sf::Vector2f(offset.x * sign, offset.y));
	projectile->setVelocity(getVelocity().x + 160.f * sign, -40.f);

	mBullets.emplace_back(projectile.get());

	node.attachChild(std::move(projectile));
}