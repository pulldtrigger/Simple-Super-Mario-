#include "Item.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <array>
#include <iostream>
#define Debug

namespace
{
	const static std::vector<ItemData>& Table = data::initializeItemData();
}

const sf::Vector2f Item::Gravity(0.f, 25.f);

Item::Item(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(None)
	, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
	, mCollisionDispatcher()
	, mCollision()
	, mBehaversCollision()
	, mUpdateDispatcher()
	, mUpdater()
{
	using namespace std::placeholders;

	switch (mType)
	{
	case Type::StaticCoin:
		mCollision = std::bind(&Item::collisions, this, _1, _2);
		mBehaversCollision.insert({
			{ Category::BigPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
		});
		break;
	case Type::MoveableCoin:
		mUpdater = std::bind(&Item::moveableCoinUpdate, this, _1);

		break;
	case Type::Mushroom:
	{
		mUpdater = std::bind(&Item::behaversUpdate, this, _1);
		mUpdateDispatcher.emplace_back(Behavors::None, std::bind(&Item::mushroomNoneUpdate, this, _1));
		mUpdateDispatcher.emplace_back(Behavors::Air, std::bind(&Item::airUpdate, this, _1));
		mUpdateDispatcher.emplace_back(Behavors::Ground, std::bind(&Item::mushroomGroundUpdate, this, _1));
		if (mUpdateDispatcher.capacity() > mUpdateDispatcher.size())
		{
			mUpdateDispatcher.shrink_to_fit();
		}
		mCollision = std::bind(&Item::resolveMushroom, this, _1, _2);
		Dispatcher airCollision({
			// Tiles
			{ Category::Brick, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			{ Category::Block, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			{ Category::TransformBox, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			{ Category::CoinsBox, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			{ Category::SoloCoinBox, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			{ Category::SolidBox, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			// Enemies
			{ Category::Goomba, std::bind(&Item::airMushroomObjectsCollision, this, _1, _2) },
			// Player
			{ Category::BigPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
		});

		Dispatcher groundCollision({
			// Tiles
			{ Category::Brick, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			{ Category::Block, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			{ Category::TransformBox, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			{ Category::CoinsBox, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			{ Category::SoloCoinBox, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			{ Category::SolidBox, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			// Enemies
			{ Category::Goomba, std::bind(&Item::groundMushroomObjectsCollision, this, _1, _2) },
			// Player
			{ Category::BigPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
		});

		mCollisionDispatcher.emplace_back(Behavors::Air, airCollision);
		mCollisionDispatcher.emplace_back(Behavors::Ground, groundCollision);
		if (mCollisionDispatcher.capacity() > mCollisionDispatcher.size())
		{
			mCollisionDispatcher.shrink_to_fit();
		}
	}
		break;
	case Type::Flower:
		mUpdater = std::bind(&Item::flowerUpdate, this, _1);
		mCollision = std::bind(&Item::collisions, this, _1, _2);
		mBehaversCollision.insert({
			{ Category::BigPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
		});
		break;
	case Type::Star:
	{
		mUpdater = std::bind(&Item::behaversUpdate, this, _1);
		mUpdateDispatcher.emplace_back(Behavors::None, std::bind(&Item::starNoneUpdate, this, _1));
		mUpdateDispatcher.emplace_back(Behavors::Air, std::bind(&Item::airUpdate, this, _1));
		if (mUpdateDispatcher.capacity() > mUpdateDispatcher.size())
		{
			mUpdateDispatcher.shrink_to_fit();
		}
		mCollision = std::bind(&Item::collisions, this, _1, _2);
		mBehaversCollision.insert({
			// Tiles
			{ Category::Brick, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			{ Category::Block, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			{ Category::TransformBox, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			{ Category::CoinsBox, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			{ Category::SoloCoinBox, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			{ Category::SolidBox, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			// Enemies
			{ Category::Goomba, std::bind(&Item::starObjectsCollision, this, _1, _2) },
			// Player
			{ Category::BigPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
			{ Category::SmallPlayer, std::bind(&Item::playerCollision, this, _1, _2) },
		});
	}
	break;
	default: break;
	}

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

unsigned int Item::getCategory() const
{
	const static std::array<unsigned int, Type::TypeCount> category
	{
		Category::StaticCoin,
		Category::MoveableCoin,
		Category::Mushroom,
		Category::Flower,
		Category::Star,
	};

	return category[mType];
}

bool Item::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Item::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Item::moveableCoinUpdate(sf::Time dt)
{
	accelerate(Gravity);

	auto vel = getVelocity();
	if (vel.y > 350.f)
	{
		destroy();
		return;
	}
}

void Item::behaversUpdate(sf::Time dt)
{
	for (const auto& behavor : mUpdateDispatcher)
	{
		if (behavor.first != mBehavors) continue;

		behavor.second(dt);
	}
}

void Item::airUpdate(sf::Time dt)
{
	accelerate(Gravity);
}

void Item::mushroomNoneUpdate(sf::Time dt)
{
	const static auto intialPosition = getWorldPosition().y;

	if (getWorldPosition().y < intialPosition - mSprite.getLocalBounds().height * 3.f / 4.f)
	{
		setVelocity({ 40.f, 0.f });
		mBehavors = Air;//Ground;
	}
}

void Item::mushroomGroundUpdate(sf::Time dt)
{
	auto vel = getVelocity();
	vel.y = std::min({}, vel.y);
	setVelocity(vel);

	if (getFootSenseCount() == 0) { mBehavors = Air; };
}

void Item::flowerUpdate(sf::Time dt)
{
	const static auto intialPosition = getWorldPosition().y;

	if (getWorldPosition().y < intialPosition - mSprite.getLocalBounds().height * 3.f / 4.f)
	{
		setVelocity({});
	}
}

void Item::starNoneUpdate(sf::Time dt)
{
	const static auto intialPosition = getWorldPosition().y;

	if (getWorldPosition().y < intialPosition - mSprite.getLocalBounds().height * 3.f / 4.f)
	{
		setVelocity({40.f, -350.f});
		mBehavors = Air;
	}
}

void Item::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (isDestroyed())
	{
		std::cout << "Item died\n";
		mIsMarkedForRemoval = true;
		return;
	}

	if (mUpdater) mUpdater(dt);

	if ( mType != Type::Mushroom)
		updateAnimation(dt);

	Entity::updateCurrent(dt, commands);
}

void Item::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
#ifdef Debug
	target.draw(mFootShape, states);
#endif // Debug
}

sf::FloatRect Item::getFootSensorBoundingRect() const
{
	return getWorldTransform().transformRect(mFootShape.getGlobalBounds());
}

void Item::setFootSenseCount(unsigned int count)
{
	mFootSenseCount = count;
}

unsigned int Item::getFootSenseCount() const
{
	return mFootSenseCount;
}

void Item::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	if (mCollision) mCollision(manifold, other);
}

void Item::collisions(const sf::Vector3f& manifold, SceneNode* other)
{
	for (const auto& collider : mBehaversCollision)
	{
		if (collider.first & other->getCategory())
		{
			collider.second(manifold, other);
		}
	}
}

void Item::resolveMushroom(const sf::Vector3f& manifold, SceneNode* other)
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

void Item::playerCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (other->isDying()) return;

	if (mType != Type::Flower)
	{
		move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
		destroy();

	}
}

void Item::airMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	mBehavors = Behavors::Ground;
}

void Item::groundMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (manifold.x != 0)
	{
		auto vel = getVelocity();
		vel.x = -vel.x;
		setVelocity(vel);
	}
}

void Item::starObjectsCollision(const sf::Vector3f& manifold, SceneNode* other)
{
	if (mBehavors != Behavors::Air) return;
	move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
	if (manifold.x != 0.f)
	{
		auto vel = getVelocity();
		vel.x = -vel.x; // jump force
		setVelocity(vel);
	}
	else
	{
		//jump away from floor,
		auto vel = getVelocity();
		vel.y = -250.f; // jump force
		setVelocity(vel);
	}
}

void Item::updateAnimation(sf::Time dt)
{
	auto textureRect = mSprite.getTextureRect();
	const static auto numFrames = 4u;
	const static auto textureOffest = 0;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 10.f;
	const static auto textureBounds = sf::Vector2i(textureRect.width * numFrames, textureRect.height);
	const auto startTexture = sf::IntRect(textureOffest, textureRect.top, textureRect.width, textureRect.height);

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