#include "Tile.hpp"
#include "ResourceHolder.hpp"
#include "ParticleNode.hpp"
#include "CommandQueue.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
//#define Debug

Tile::Tile(Type type, const TextureHolder& textures, sf::Vector2f size)
	: mType(type)
	, mSprite(textures.get(Textures::Tile), (type == Type::Box) ? sf::IntRect(16 * 24, 0, 16, 16) : sf::IntRect(16, 0, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mIsHitBySmallPlayer(false)
	, mIsHitByBigPlayer(false)
	, mTimer(sf::Time::Zero)
	, mJump(0.f, -5.2f)
	, mSpawnedExplosion(false)
	, mElapsedTime(sf::Time::Zero)
	, mIdleRect(sf::IntRect(16 * 27, 0, 16, 16))
	, mCanAnimate(true)
	, mItem(Item::None)
	, mCoinsCount()
{
	if (mType != Type::Block)
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
	else
	{
		mFootShape.setSize(size);
		mFootShape.setFillColor(sf::Color::Transparent);
		mFootShape.setOutlineColor(sf::Color::Red);
		mFootShape.setOutlineThickness(-0.5f);
		auto bounds = mFootShape.getLocalBounds();
		mFootShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	}
}

void Tile::setItem(Item item)
{
	mItem = item;
}

void Tile::setCoinsCount(unsigned int count)
{
	mCoinsCount = count;
}

unsigned int Tile::getCategory() const
{
	return Category::Tile;
}

bool Tile::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Tile::getBoundingRect() const
{
	if(mType != Type::Block)
		return getWorldTransform().transformRect(mSprite.getGlobalBounds());
	else
		return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Tile::updateCurrent(sf::Time dt, CommandQueue& commands)
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

		if (mType != Type::Brick)
		{
			switch (mItem)
			{
			case Item::Coin:
				if (mCoinsCount > 0) break;
				mCanAnimate = false;
				mSprite.setTextureRect(mIdleRect);
				mItem = None;
				break;
			case Item::TransformMushroom:
				mCanAnimate = false;
				mSprite.setTextureRect(mIdleRect);
				mItem = None;
				// TODO: create mushroom
				break;
			default:break;
			}
		}
	}

	updateAnimation(dt);
}

void Tile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if(mType != Type::Block)
		target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

sf::FloatRect Tile::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Tile::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Tile::getFootSenseCount() const
{
	return mFootSenseCount;
}

Type Tile::getType() const
{
	return mType;
}

void Tile::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (other->isDying()) break;
		if (mType == Type::Brick)
		{
			if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
			{
				mIsHitBySmallPlayer = true;
				mTimer = sf::Time::Zero;
				move(mJump);
			}
		}
		else
		{
			switch (mItem)
			{
			case Item::Coin:
				if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
				{
					mCoinsCount--;
					mIsHitBySmallPlayer = true;
					mTimer = sf::Time::Zero;
					move(mJump);
				}
				break;
			case Item::TransformMushroom:
				if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
				{
					mIsHitBySmallPlayer = true;
					mTimer = sf::Time::Zero;
					move(mJump);
				}
				break;
			default:break;
			}
		}
		break;
	case Type::BigPlayer:
		if (mType == Type::Brick)
		{
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
		}
		else
		{
			switch (mItem)
			{
			case Item::Coin:
				if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
				{
					mCoinsCount--;
					mIsHitBySmallPlayer = true;
					mTimer = sf::Time::Zero;
					move(mJump);
				}
				break;
			case Item::TransformMushroom:
				if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
				{
					mIsHitBySmallPlayer = true;
					mTimer = sf::Time::Zero;
					move(mJump);
				}

				break;
			default:break;
			}
		}
		break;
	case Type::Goomba:
		if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
		break;
	case Type::Block:
	case Type::Brick:
	case Type::Box:
		break;
	default: break;
	}
}

void Tile::checkExplosion(CommandQueue& commands)
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

void Tile::updateAnimation(sf::Time dt)
{
	if (mType != Type::Box || !mCanAnimate) return;

	auto textureRect = mSprite.getTextureRect();

	const static auto numFrames = 3u;
	const static auto textureOffest = textureRect.left;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 5.f;
	const static auto textureBounds = sf::Vector2i(textureRect.width * numFrames, textureRect.height);
	const static auto startTexture = textureRect;

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