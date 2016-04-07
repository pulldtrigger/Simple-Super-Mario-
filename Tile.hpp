#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Tile final : public Entity
{
public:
	explicit Tile(Type type, const TextureHolder& textures, sf::Vector2f size = {});

	void setCoinsCount(unsigned int count);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* other) override;
	void resolveBrick(const sf::Vector3f& manifold, SceneNode* other);
	void resolveCoinsBox(const sf::Vector3f& manifold, SceneNode* other);
	void resolveSoloCoinBox(const sf::Vector3f& manifold, SceneNode* other);
	void resolveTransformBox(const sf::Vector3f& manifold, SceneNode* other);

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;

	void checkExplosion(CommandQueue& commands);
	void updateAnimation(sf::Time dt);
	void setup(sf::Vector2f size);

	void createItem(SceneNode& node, const TextureHolder& textures, Type type);


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

	unsigned int mCoinsCount;

	Command mCoinCommand;
	Command mTransformCommand;
	bool mIsFired;
};
