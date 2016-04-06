#include "Player.hpp"
#include "ResourceHolder.hpp"
#include "CommandQueue.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <functional>

#define Debug


Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(Air)
	, mSprite(textures.get(Textures::Player), sf::IntRect(80, 32, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mJumpRect(sf::IntRect(80 + (16 * 5), 32, 16, 16))
	, mDirectionRect(sf::IntRect(80 + (16 * 4), 32, 16, 16))
	, mIdleRect(sf::IntRect(80, 32, 16, 16))
	, mDirectionTime(sf::Time::Zero)
	, mCurrentDirection(Right | Up)
	, mPreviousDirection(Right | Up)
	, isChangingDirection(false)
	, isRightFace(true)
	, mAbilities(Regular)
	, mFireCommand()
	, mIsFiring(false)
	, mBullets()
	, mTimer(sf::Time::Zero)
	, mAffects(Blinking)
	, mScaleToggle(true)
	, mIsDying(false)
	, mIsSmallPlayerTransformed(false)
{
	setup();

	mFireCommand.category = Category::SceneMainLayer;
	mFireCommand.action = std::bind(&Player::createProjectile, this, std::placeholders::_1, std::cref(textures));
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

unsigned int Player::getCategory() const
{
	return Category::Player;
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

void Player::applyFireable(Type type = Type::BigPlayer, unsigned int ability)
{
	mAbilities |= ability;
	mJumpRect = (type == Type::BigPlayer) ? sf::IntRect(80 + (16 * 5), 0 + 96, 16, 32) : sf::IntRect(80 + (16 * 5), 32 + 96, 16, 16);
	mDirectionRect = (type == Type::BigPlayer) ? sf::IntRect(80 + (16 * 4), 0 + 96, 16, 32) : sf::IntRect(80 + (16 * 4), 32 + 96, 16, 16);
	mIdleRect = (type == Type::BigPlayer) ? sf::IntRect(80, 0 + 96, 16, 32) : sf::IntRect(80, 32 + 96, 16, 16);
	mSprite.setTextureRect(mIdleRect);
	if (mType != type)
		setup();
}

void Player::applyTransformation(Type type, unsigned int affector)
{
	mType = type;
	mJumpRect = (type == Type::BigPlayer) ? sf::IntRect(80 + (16 * 5), 0, 16, 32) : sf::IntRect(80 + (16 * 5), 32, 16, 16);
	mDirectionRect = (type == Type::BigPlayer) ? sf::IntRect(80 + (16 * 4), 0, 16, 32) : sf::IntRect(80 + (16 * 4), 32, 16, 16);
	mIdleRect = (type == Type::BigPlayer) ? sf::IntRect(80, 0, 16, 32) : sf::IntRect(80, 32, 16, 16);
	mSprite.setTextureRect(mIdleRect);
	setup();

	if (!(mAffects & (Scaling | Death)))
		mAffects = affector;
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

	mTimer += dt;
	if (mAffects & Death)
	{
		if (mTimer <= sf::seconds(1.5f)) return;
		applyTransformation(Type::SmallPlayer);
		mAffects = Blinking;
		mTimer = sf::Time::Zero;
		return;
	}

	if (mTimer <= sf::seconds(1.f)) return;

	if (mAffects & Scaling)
		mSprite.setScale({ 1.f,  1.f });

	if (mAffects & Blinking)
		mSprite.setColor(sf::Color::White);

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

	switch (mBehavors)
	{
	case Behavors::Air:
	{
		auto vel = getVelocity();
		vel.x *= 0.8f;
		if (mIsSmallPlayerTransformed) vel.y = 0.f;
		setVelocity(vel);

		auto displacement = static_cast<int>(vel.x * dt.asSeconds());

		if (displacement > 0)
		{
			mCurrentDirection &= ~(Left);
			mCurrentDirection |= Right;
		}
		else if (displacement < 0)
		{
			mCurrentDirection &= ~(Right);
			mCurrentDirection |= Left;
		}

	}
	break;
	case Behavors::Ground:
	{
		auto vel = getVelocity();
		if (vel.y > 0.f) vel.y = 0.f;
		vel.x *= 0.83f;
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
	break;
	case Behavors::Dying:
	{
		auto vel = getVelocity();
		vel.x = 0.f;
		setVelocity(vel);
	}
	break;
	default:break;
	}

	playEffects(dt);

	if (mAffects & Death) return;

	updateDirection(dt);

	updateAnimation(dt);

	checkProjectiles();

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
		case Type::Solid:
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

				mBehavors = Ground;
				mCurrentDirection &= ~(Up);
				mCurrentDirection |= Idle;
			}
			break;
		case Type::Goomba:
			if (other->isDying()) break;
			if (mIsSmallPlayerTransformed) return;
			if (mAbilities & Abilities::Invincible) break;
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			if (manifold.x != 0) // side collision
			{
				if (mType == Type::BigPlayer)
				{
					mAffects = Death | Pause | Scaling;
					mIsSmallPlayerTransformed = true;
					mAbilities = Regular;
				}
				else
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
				}
			}
			// NOTE: for some reasons this doesn't work, better to implement it on goomba side
			//else //if (manifold.y != 0)
			//{
			//	auto vel = getVelocity();
			//	vel.y = -vel.y;//-380.f;
			//	setVelocity(vel);
			//}
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
		case Type::Goomba:
			if (other->isDying()) break;
			if (mIsSmallPlayerTransformed) return;
			if (mAbilities & Abilities::Invincible) break;
			if (manifold.x != 0)
			{
				if (mType == Type::BigPlayer)
				{
					mAffects = Death | Pause | Scaling | Blinking;
					mIsSmallPlayerTransformed = true;
					mAbilities = Regular;
				}
				else
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
				}
			}
			break;
		default: break;
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
	const static auto numFrames = 3u;
	const static auto textureOffest = textureRect.left + textureRect.width;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 10.f;
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
	using namespace std::placeholders;

	mBullets.erase(std::remove_if(mBullets.begin(), mBullets.end(), std::mem_fn(&Projectile::isDestroyed)), mBullets.end());

	if (mCurrentDirection & Idle) return;

	std::for_each(mBullets.begin(), mBullets.end(), std::bind(&Projectile::adaptProjectileVelocity, _1, getVelocity().x));
}

void Player::createProjectile(SceneNode& node, const TextureHolder& textures)
{
	auto projectile(std::make_unique<Projectile>(Type::Projectile, textures));

	const sf::Vector2f offset(mSprite.getLocalBounds().width / 2.f, -mSprite.getLocalBounds().height / 2.f);

	auto sign = (isRightFace) ? 1.f: -1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	projectile->setVelocity(160.f * sign, -40.f);

	mBullets.emplace_back(projectile.get());

	node.attachChild(std::move(projectile));
}