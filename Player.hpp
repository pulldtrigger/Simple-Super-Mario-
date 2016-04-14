#pragma once


#include "Command.hpp"
#include "Projectile.hpp"

#include <SFML/Graphics/RectangleShape.hpp>


class Player final : public Entity
{
public:
	enum Type
	{
		SmallPlayer,
		BigPlayer,
		TypeCount
	};

	enum Abilities
	{
		None		= 0,
		Regular		= 1 << 0,
		Transform	= 1 << 1,
		Fireable	= 1 << 2,
		Invincible	= 1 << 3,
	};


private:
	enum Behavors
	{
		Air,
		Ground,
		Dying
	};

	enum Direction
	{
		Idle	= 1 << 0,
		Right	= 1 << 1,
		Left	= 1 << 2,
		Up		= 1 << 3,
	};

	enum Affects
	{
		Nothing		= 1 << 0,
		Pause		= 1 << 1,
		Blinking	= 1 << 2,
		Scaling		= 1 << 3,
		Shifting	= 1 << 4,
		Death		= 1 << 5,
		Power		= 1 << 6,
	};

	using DispatchHolder = std::vector<std::pair<Behavors, Dispatcher>>;
	using FunctionUpdater = std::function<void(sf::Time)>;
	using UpdateHolder = std::vector<std::pair<Behavors, FunctionUpdater>>;


public:
	explicit Player(Type type, const TextureHolder& textures);

	void applyForce(sf::Vector2f velocity);
	void fire();
	bool paused();

	void applyTransformation(Type type = Type::BigPlayer);
	void applyFireable();
	void applyInvincible();
	void applyBigPlayerShifting(sf::Time dt);
	void applySmallPlayerShifting(sf::Time dt);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;
	unsigned int getAbilities() const override;

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;

	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;

	void updateAnimation(sf::Time dt);
	void updateDirection(sf::Time dt);

	void checkProjectiles();
	void checkProjectileLaunch(sf::Time dt, CommandQueue& commands);
	void createProjectile(SceneNode& node, const TextureHolder& textures);

	void setup();

	bool scalingEffect(sf::Time dt, sf::Vector2f targetScale);
	void playEffects(sf::Time dt);
	bool isDying() const override;

	void initialDispatching();

	void airTileCollision(const sf::Vector3f& manifold, SceneNode* other);
	void airEnemyCollision(const sf::Vector3f& manifold, SceneNode* other);

	void mushroomCollision(const sf::Vector3f& manifold, SceneNode* other);
	void flowerCollision(const sf::Vector3f& manifold, SceneNode* other);
	void starCollision(const sf::Vector3f& manifold, SceneNode* other);

	void groundTileCollision(const sf::Vector3f& manifold, SceneNode* other);
	void groundEnemyCollision(const sf::Vector3f& manifold, SceneNode* other);

	void airUpdate(sf::Time dt);
	void groundUpdate(sf::Time dt);
	void dyingUpdate(sf::Time dt);


private:
	Type mType;
	Behavors mBehavors;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;

	sf::Time mElapsedTime;
	sf::IntRect mJumpRect; // TODO: remove these
	sf::IntRect mDirectionRect;
	sf::IntRect mIdleRect;

	sf::Time mDirectionTime;
	unsigned int mCurrentDirection;
	unsigned int mPreviousDirection;
	bool isChangingDirection;
	bool isRightFace;

	unsigned int mAbilities;
	sf::Time mAbilitiesTime;
	Command mFireCommand;
	bool mIsFiring;
	std::vector<Projectile*> mBullets;

	sf::Time mTimer;
	unsigned int mAffects;
	bool mScaleToggle;

	bool mIsDying;
	bool mIsSmallPlayerTransformed;

	DispatchHolder mCollisionDispatcher;
	UpdateHolder mUpdateDispatcher;
};