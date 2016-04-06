#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Tile final : public Entity
{
public:
	enum Item
	{
		None,
		Coin,
		TransformMushroom,
	};


public:
	explicit Tile(Type type, const TextureHolder& textures, sf::Vector2f size = {});

	void setItem(Item item);
	void setCoinsCount(unsigned int count);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;
	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;

	void checkExplosion(CommandQueue& commands);
	void updateAnimation(sf::Time dt);


private:
	Type mType;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;
	bool mIsHitBySmallPlayer;
	bool mIsHitByBigPlayer;
	sf::Time mTimer;
	sf::Vector2f mJump;

	bool mSpawnedExplosion;

	sf::Time mElapsedTime;
	sf::IntRect mIdleRect;
	bool mCanAnimate;

	Item mItem;
	unsigned int mCoinsCount;
};
