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
		Mushroom,
		Flower,
		Star,
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
	void collisions(const sf::Vector3f& manifold, SceneNode* other);
	void resolveMushroom(const sf::Vector3f& manifold, SceneNode* other);

	void updateAnimation(sf::Time dt);

	void moveableCoinUpdate(sf::Time dt);

	void behaversUpdate(sf::Time dt);
	void airUpdate(sf::Time dt);

	void mushroomNoneUpdate(sf::Time dt);
	void mushroomGroundUpdate(sf::Time dt);

	void flowerUpdate(sf::Time dt);

	void starNoneUpdate(sf::Time dt);

	void playerCollision(const sf::Vector3f& manifold, SceneNode* other);

	void airMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);
	void groundMushroomObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);

	void starObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);


private:
	static const sf::Vector2f Gravity;

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

