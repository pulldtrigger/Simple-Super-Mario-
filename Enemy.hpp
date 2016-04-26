#pragma once


#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Enemy final : public Entity
{
public:
	enum Type
	{
		Goomba,
		Troopa,
		Shell,
		Plant,
		TypeCount
	};


private:
	enum Behavors
	{
		Air,
		Ground,
		Dying
	};

	using DispatchHolder = std::vector<std::pair<Behavors, Dispatcher>>;
	using FunctionUpdater = std::function<void(sf::Time)>;
	using UpdateHolder = std::vector<std::pair<Behavors, FunctionUpdater>>;


public:
	explicit Enemy(Type type, const TextureHolder& textures);


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
	void resolveEnemy(const sf::Vector3f& manifold, SceneNode* other);

	void airPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void groundPlayerCollision(const sf::Vector3f& manifold, SceneNode* other);

	void airObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);
	void groundObjectsCollision(const sf::Vector3f& manifold, SceneNode* other);

	void projectileCollision(const sf::Vector3f& manifold, SceneNode* other);

	void die() override;
	bool isDying() const override;

	void updateAnimation(sf::Time dt);

	void behaversUpdate(sf::Time dt);
	void groundUpdate(sf::Time dt);
	void dyingUpdate(sf::Time dt);

	void setUp();

private:
	static const sf::Vector2f Gravity;

	Type mType;
	Behavors mBehavors;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;

	sf::Time mElapsedTime;
	sf::Time mDyingTimer;
	bool mIsDying;
	bool mIsCrushed;

	UpdateHolder mUpdateDispatcher;
	FunctionUpdater mUpdater;

	DispatchHolder mCollisionDispatcher;
	Function mCollision;
	Dispatcher mBehaversCollision;
	//DeathHolder mDeathDispatcher;
};
