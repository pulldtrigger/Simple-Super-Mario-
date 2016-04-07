#include "Item.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>
//#define Debug

Item::Item(Type type, const TextureHolder& textures)
	: mType(type)
	, mBehavors(None)
	, mSprite(textures.get(Textures::Items), (type == Type::MoveableCoin) 
		? sf::IntRect(0, 16 * 7, 16, 16) 
		: (type == Type::TransformMushroom) 
		? sf::IntRect(0, 0, 16, 16) 
		: sf::IntRect(0, 16 * 6, 16, 16))
	, mFootSenseCount()
	, mIsMarkedForRemoval(false)
	, mElapsedTime(sf::Time::Zero)
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
}

unsigned int Item::getCategory() const
{
	return Category::Item;
}

bool Item::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Item::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Item::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	static const sf::Vector2f Gravity(0.f, 25.f);

	if (isDestroyed())
	{
		std::cout << "Item died\n";
		mIsMarkedForRemoval = true;
		return;
	}

	switch (mType)
	{
	case Type::StaticCoin:
	{
		updateAnimation(dt);
	}
	break;
	case Type::MoveableCoin:
	{
		accelerate(Gravity);
		// just fall
		auto vel = getVelocity();
		if (vel.y > 350.f)
		{
			destroy();
			return;
		}
		updateAnimation(dt);
	}
	break;
	case Type::TransformMushroom:
	{
		switch (mBehavors)
		{
		case Behavors::None:
		{
			auto vel = getVelocity();

			const static auto intialPosition = getWorldPosition().y;

			if (getWorldPosition().y < intialPosition - mSprite.getLocalBounds().height * 3.f / 4.f)
			{
				vel.y = 0.f;
				vel.x = 40.f;
				setVelocity(vel);
				mBehavors = Air;//Ground;
			}
		}
		break;
		case Behavors::Air:
		{
			// just fall
			accelerate(Gravity);
		}
		break;
		case Behavors::Ground:
		{
			auto vel = getVelocity();
			if (vel.y > 0.f) vel.y = 0.f;
			setVelocity(vel);

			if (getFootSenseCount() == 0) { mBehavors = Air; };
		}
		break;
		default: break;
		}
	}
	break;
	default:break;
	}

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

Type Item::getType() const
{
	return mType;
}

void Item::resolve(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (mType)
	{
	case Type::StaticCoin:
		switch (other->getType())
		{
		case Type::SmallPlayer:
		case Type::BigPlayer:
			if (other->isDying()) break;
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			destroy();
			break;
		default: break;
		}
		break;

	case Type::TransformMushroom:
		resolveTransformMushroom(manifold, other);
		break;
	default:break;
	}
}

void Item::resolveTransformMushroom(const sf::Vector3f& manifold, SceneNode* other)
{
	switch (mBehavors)
	{
	case Behavors::Air:
		switch (other->getType())
		{
		case Type::Block:
		case Type::Brick:
		case Type::SolidBox:
		case Type::CoinsBox:
		case Type::SoloCoinBox:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			mBehavors = Behavors::Ground;
			break;
		case Type::SmallPlayer:
		case Type::BigPlayer:
			if (other->isDying()) break;
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			destroy();
			break;
		default: break;
		}

		break;
	case Behavors::Ground:
		switch (other->getType())
		{
		case Type::Block:
		case Type::Brick:
		case Type::CoinsBox:
		case Type::SoloCoinBox:
		case Type::SolidBox:
		case Type::Goomba:
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			if (manifold.x != 0)
			{
				auto vel = getVelocity();
				vel.x = -vel.x;
				setVelocity(vel);
			}
			break;
		case Type::SmallPlayer:
		case Type::BigPlayer:
			if (other->isDying()) break;
			move(sf::Vector2f(manifold.x, manifold.y) * manifold.z);
			destroy();
			break;
		default: break;
		}

		break;
	default: break;
	}
}

void Item::updateAnimation(sf::Time dt)
{
	auto textureRect = mSprite.getTextureRect();
	const static auto numFrames = 4u;
	const static auto textureOffest = 0;
	const static auto animationInterval = sf::seconds(1.f);
	const static auto animateRate = 15.f;
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