#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Command.hpp"
#include "Item.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Tile final : public Entity
{
public:
	enum Type
	{
		Block,
		Brick,
		SoloCoinBox,
		CoinsBox,
		TransformBox,
		FireBox,
		ShiftBox,
		SolidBox,
		TypeCount
	};


private:
	using DispatchHolder = std::vector<std::pair<Type, Dispatcher>>;
	using FunctionUpdater = std::function<void(sf::Time, CommandQueue&)>;


public:
	explicit Tile(Type type, const TextureHolder& textures, sf::Vector2f size = {});

	void setCoinsCount(unsigned int count);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	void resolve(const sf::Vector3f& manifold, SceneNode* other) override;

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;

	void checkExplosion(CommandQueue& commands);
	void updateAnimation(sf::Time dt);
	void setup(sf::Vector2f size);

	void createItem(SceneNode& node, const TextureHolder& textures, Item::Type type);

	void brickUpdate(sf::Time dt, CommandQueue& commands);
	void boxUpdate(sf::Time dt, CommandQueue& commands);

	void brickBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void brickSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);

	void coinsBoxBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void coinsBoxSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);

	void boxBigPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void boxSmallPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void enemyCollision(const sf::Vector3f& manifold, SceneNode* other);


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
	bool mCanAnimate;

	unsigned int mCoinsCount;

	Command mCommand;
	bool mIsFired;

	Dispatcher mCollisionDispatcher;
	FunctionUpdater mUpdater;
};
