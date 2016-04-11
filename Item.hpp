#pragma once


#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Item final : public Entity
{
public:
	enum Type
	{
		StaticCoin,
		MoveableCoin,
		TransformMushroom,
		TypeCount
	};


private:
	enum Behavors
	{
		None,
		Air,
		Ground
	};


	using DispatchHolder = std::vector<std::pair<Behavors, Dispatcher>>;
	using FunctionUpdater = std::function<void(sf::Time)>;
	using UpdateHolder = std::vector<std::pair<Behavors, FunctionUpdater>>;


public:
	explicit Item(Type type, const TextureHolder& textures);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;

	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;
	void resolveStaticCoin(const sf::Vector3f& manifold, SceneNode* other);
	void resolveTransformMushroom(const sf::Vector3f& manifold, SceneNode* other);

	void updateAnimation(sf::Time dt);

	void staticCoinUpdate(sf::Time dt);
	void moveableCoinUpdate(sf::Time dt);
	void transformMushroomUpdate(sf::Time dt);
	void transformMushroomNoneUpdate(sf::Time dt);
	void transformMushroomAirUpdate(sf::Time dt);
	void transformMushroomGroundUpdate(sf::Time dt);

	void playerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void airTransformMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);
	void groundTransformMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);


private:
	Type mType;
	Behavors mBehavors;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;

	sf::Time mElapsedTime;

	DispatchHolder mCollisionDispatcher;
	Function mCollision;
	Dispatcher mBehaversCollision;
	UpdateHolder mUpdateDispatcher;
	FunctionUpdater mUpdater;
};

