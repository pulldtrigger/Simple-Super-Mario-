#include "Tile.hpp"
#include "ResourceHolder.hpp"
#include "ParticleNode.hpp"
#include "CommandQueue.hpp"
#include "Item.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
#define Debug

Tile::Tile(Type type, const TextureHolder& textures, sf::Vector2f size)
	: mType(type)
	, mSprite(textures.get(Textures::Tile), (type == Type::Brick) ? sf::IntRect(16, 0, 16, 16) : sf::IntRect(16 * 24, 0, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mIsHitBySmallPlayer(false)
	, mIsHitByBigPlayer(false)
	, mTimer(sf::Time::Zero)
	, mJump(0.f, -5.2f)
	, mSpawnedExplosion(false)
	, mElapsedTime(sf::Time::Zero)
	, mIdleRect(sf::IntRect(16 * 27, 0, 16, 16))
	, mCanAnimate((type == Type::Brick || type == Type::Block) ? false : true)
	, mCoinsCount()
	, mCoinCommand()
	, mTransformCommand()
	, mIsFired(false)
{
	switch (mType)
	{
	case Type::CoinsBox:
	case Type::SoloCoinBox:
		mCoinCommand.category = Category::BackLayer;
		mCoinCommand.action = std::bind(&Tile::createItem, this, std::placeholders::_1, std::cref(textures), Type::MoveableCoin);
	break;
	case Type::TransformBox:
		mTransformCommand.category = Category::BackLayer;
		mTransformCommand.action = std::bind(&Tile::createItem, this, std::placeholders::_1, std::cref(textures), Type::TransformMushroom);
	break;
	default: break;
	}

	setup(size);
}

void Tile::setup(sf::Vector2f size)
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
		std::cout << "Tile destroyed\n";
		if (mType == Type::Brick)
			checkExplosion(commands);
		mIsMarkedForRemoval = true;
		return;
	}

	mTimer += dt;

	if (mTimer >= sf::seconds(0.0225f) && mIsHitByBigPlayer)
	{
		switch (mType)
		{
		case Type::Brick:
			move(-mJump);
			destroy();
			mIsHitByBigPlayer = false;
			break;
		case Type::CoinsBox:
		case Type::SoloCoinBox:
		case Type::TransformBox:
		default: break;
		}
	}

	if (mTimer >= sf::seconds(0.25f) && mIsHitBySmallPlayer)
	{
		switch (mType)
		{
		case Type::Brick:
			move(-mJump);
			mIsHitBySmallPlayer = false;
			break;
		case Type::CoinsBox:
			move(-mJump);
			mIsHitBySmallPlayer = false;
			if (mCoinsCount > 0) break;
			mCanAnimate = false;
			mSprite.setTextureRect(mIdleRect);
			mType = Type::SolidBox;
			break;
		case Type::SoloCoinBox:
		case Type::TransformBox:
			move(-mJump);
			mIsHitBySmallPlayer = false;
			mCanAnimate = false;
			mSprite.setTextureRect(mIdleRect);
			mType = Type::SolidBox;
			break;
		case Type::Block:
			break;
		default: break;
		}
	}

	// check commands
	switch (mType)
	{
	case Type::Brick:
		break;
	case Type::CoinsBox:
	case Type::SoloCoinBox:
		if (mIsFired)
		{
			mIsFired = false;
			commands.push(mCoinCommand);
		}
		break;
	case Type::TransformBox:
		if (mIsFired)
		{
			mIsFired = false;
			commands.push(mTransformCommand);
		}
		break;
	case Type::Block:
		break;
	default: break;
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
	switch (mType)
	{
	case Type::Brick:
		resolveBrick(manifold, other);
		break;
	case Type::CoinsBox:
		resolveCoinsBox(manifold, other);
		break;
	case Type::SoloCoinBox:
		resolveSoloCoinBox(manifold, other);
		break;
	case Type::TransformBox:
		resolveTransformBox(manifold, other);
		break;
	case Type::Block:
		break;
	default: break;
	}
}

void Tile::resolveBrick(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (other->isDying()) break;

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
	default: break;
	}
}

void Tile::resolveCoinsBox(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (other->isDying()) break;
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
		{
			mCoinsCount--;
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			mIsFired = true;
			move(mJump);
		}
		break;
	case Type::BigPlayer:
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
		{
			mCoinsCount--;
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			mIsFired = true;
			move(mJump);
		}
		break;
	case Type::Goomba:
		if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
		break;
	default: break;
	}
}

void Tile::resolveSoloCoinBox(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (other->isDying()) break;
	case Type::BigPlayer:
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
		{
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			mIsFired = true;
			move(mJump);
		}
		break;
	case Type::Goomba:
		if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
		break;
	default: break;
	}
}

void Tile::resolveTransformBox(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (other->getType())
	{
	case Type::SmallPlayer:
		if (other->isDying()) break;
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
		{
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			mIsFired = true;
			move(mJump);
		}
		break;
	case Type::BigPlayer:
		if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
		{
			mIsHitBySmallPlayer = true;
			mTimer = sf::Time::Zero;
			mIsFired = true;
			move(mJump);
		}
		break;
	case Type::Goomba:
		if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
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
	if (!mCanAnimate) return;

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

void Tile::createItem(SceneNode& node, const TextureHolder& textures, Type type)
{
	switch (type)
	{
	case Type::MoveableCoin:
	{
		auto item(std::make_unique<Item>(type, textures));
		item->setPosition(getWorldPosition());
		item->setVelocity(0.f, -475.f);
		node.attachChild(std::move(item));
	}
	break;
	case Type::TransformMushroom:
	{
		auto item(std::make_unique<Item>(type, textures));
		item->setPosition(getWorldPosition());
		item->setVelocity(0.f, -5.5f);
		node.attachChild(std::move(item));
	}
	break;
	default:break;
	}
}