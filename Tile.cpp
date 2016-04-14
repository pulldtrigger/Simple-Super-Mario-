#include "Tile.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "ParticleNode.hpp"
#include "CommandQueue.hpp"
//#include "Item.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <iostream>
#define Debug

namespace
{
	using namespace std::placeholders;

	const static std::vector<TileData>& Table = data::initializeTileData();
}

Tile::Tile(Type type, const TextureHolder& textures, sf::Vector2f size)
	: mType(type)
	, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mIsHitBySmallPlayer(false)
	, mIsHitByBigPlayer(false)
	, mTimer(sf::Time::Zero)
	, mJump(0.f, -5.2f)
	, mSpawnedExplosion(true)
	, mElapsedTime(sf::Time::Zero)
	, mCanAnimate(Table[type].canAnimate)
	, mCoinsCount()
	, mCommand()
	, mIsFired(false)
	, mCollisionDispatcher()
	, mUpdater()
{
	switch (mType)
	{
	case Type::Brick:
		mUpdater = std::bind(&Tile::brickUpdate, this, _1, _2);
		mSpawnedExplosion = false;
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::brickBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::brickSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
		break;
	case Type::CoinsBox:
		mUpdater = std::bind(&Tile::boxUpdate, this, _1, _2);
		mCommand.category = Category::BackLayer;
		mCommand.action = std::bind(&Tile::createItem, this, _1, std::cref(textures), Item::MoveableCoin);
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::coinsBoxBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::coinsBoxSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
		break;
	case Type::SoloCoinBox:
		mUpdater = std::bind(&Tile::boxUpdate, this, _1, _2);
		mCommand.category = Category::BackLayer;
		mCommand.action = std::bind(&Tile::createItem, this, _1, std::cref(textures), Item::MoveableCoin);
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::boxBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::boxSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
	break;
	case Type::TransformBox:
		mUpdater = std::bind(&Tile::boxUpdate, this, _1, _2);
		mCommand.category = Category::BackLayer;
		mCommand.action = std::bind(&Tile::createItem, this, _1, std::cref(textures), Item::Mushroom);
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::boxBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::boxSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
	break;
	case Type::FireBox:
		mUpdater = std::bind(&Tile::boxUpdate, this, _1, _2);
		mCommand.category = Category::BackLayer;
		mCommand.action = std::bind(&Tile::createItem, this, _1, std::cref(textures), Item::Flower);
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::boxBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::boxSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
		break;
	case Type::ShiftBox:
		mUpdater = std::bind(&Tile::boxUpdate, this, _1, _2);
		mCommand.category = Category::BackLayer;
		mCommand.action = std::bind(&Tile::createItem, this, _1, std::cref(textures), Item::Star);
		mCollisionDispatcher.insert({
			{ Category::BigPlayer, std::bind(&Tile::boxBigPlayerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Tile::boxSmallPlayerCollision, this, _1, _2) },
			{ Category::Goomba, std::bind(&Tile::enemyCollision, this, _1, _2) },
		});
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
	const static std::array<unsigned int, Type::TypeCount> category
	{
		Category::Block,
		Category::Brick,
		Category::SoloCoinBox,
		Category::CoinsBox,
		Category::TransformBox,
		Category::FireBox,
		Category::ShiftBox,
		Category::SolidBox,
	};

	return category[mType];
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

void Tile::brickUpdate(sf::Time dt, CommandQueue& commands)
{
	mTimer += dt;

	if (mTimer >= sf::seconds(0.25f) && mIsHitBySmallPlayer)
	{
		move(-mJump);
		mIsHitBySmallPlayer = false;
	}

	if (mTimer >= sf::seconds(0.0225f) && mIsHitByBigPlayer)
	{
		move(-mJump);
		destroy();
		mIsHitByBigPlayer = false;
	}
}

void Tile::boxUpdate(sf::Time dt, CommandQueue& commands)
{
	mTimer += dt;

	if (mTimer >= sf::seconds(0.25f) && mIsHitBySmallPlayer)
	{
		move(-mJump);
		mIsHitBySmallPlayer = false;
		if (mCoinsCount > 0) return;
		mCanAnimate = false;
		mSprite.setTextureRect(Table[mType].idleRect);
		mCollisionDispatcher.clear();
		mType = Type::SolidBox;
	}

	if (mIsFired)
	{
		mIsFired = false;
		commands.push(mCommand);
	}
}

void Tile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isDestroyed())
	{
		std::cout << "Tile destroyed\n";
		checkExplosion(commands);
		mIsMarkedForRemoval = true;
		return;
	}

	if (mUpdater) mUpdater(dt, commands);

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

void Tile::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	for (const auto& collider : mCollisionDispatcher)
	{
		if (collider.first & other->getCategory())
		{
			collider.second(manifold, other);
		}
	}
}

void Tile::brickBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
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

void Tile::brickSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;

	if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
	{
		mIsHitBySmallPlayer = true;
		mTimer = sf::Time::Zero;
		move(mJump);
	}
}

void Tile::coinsBoxBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
	{
		mCoinsCount--;
		mIsHitBySmallPlayer = true;
		mTimer = sf::Time::Zero;
		mIsFired = true;
		move(mJump);
	}
}

void Tile::coinsBoxSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;
	if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer && mCoinsCount > 0)
	{
		mCoinsCount--;
		mIsHitBySmallPlayer = true;
		mTimer = sf::Time::Zero;
		mIsFired = true;
		move(mJump);
	}
}

void Tile::boxBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
	{
		mIsHitBySmallPlayer = true;
		mTimer = sf::Time::Zero;
		mIsFired = true;
		move(mJump);
	}
}

void Tile::boxSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;
	if (manifold.y * manifold.z < 0 && !mIsHitBySmallPlayer)
	{
		mIsHitBySmallPlayer = true;
		mTimer = sf::Time::Zero;
		mIsFired = true;
		move(mJump);
	}
}

void Tile::enemyCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (mIsHitBySmallPlayer || mIsHitByBigPlayer) other->die();
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

void Tile::createItem(SceneNode& node, const TextureHolder& textures, Item::Type type)
{
	switch (type)
	{
	case Item::MoveableCoin:
	{
		auto item(std::make_unique<Item>(type, textures));
		item->setPosition(getWorldPosition());
		item->setVelocity(0.f, -475.f);
		node.attachChild(std::move(item));
	}
	break;
	case Item::Mushroom:
	{
		auto item(std::make_unique<Item>(type, textures));
		item->setPosition(getWorldPosition());
		item->setVelocity(0.f, -5.5f);
		node.attachChild(std::move(item));
	}
	break;
	case Item::Flower:
	{
		auto item(std::make_unique<Item>(type, textures));
		item->setPosition(getWorldPosition());
		item->setVelocity(0.f, -5.5f);
		node.attachChild(std::move(item));
	}
	break;
	case Item::Star:
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