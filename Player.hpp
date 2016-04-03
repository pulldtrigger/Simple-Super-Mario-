#pragma once


#include "Command.hpp"
#include "Projectile.hpp"

#include <SFML/Graphics/RectangleShape.hpp>


class Player final : public Entity
{
public:
	enum Abilities
	{
		None		= 0,
		Regular		= 1 << 0,
		Fireable	= 1 << 1,
		Invincible	= 1 << 2
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


public:
	explicit Player(Type type, const TextureHolder& textures);

	void applyForce(sf::Vector2f velocity);
	void fire();


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
	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;
	bool isDying() const override;

	void updateAnimation(sf::Time dt);
	void updateDirection(sf::Time dt);

	void checkProjectiles();
	void checkProjectileLaunch(sf::Time dt, CommandQueue& commands);
	void createProjectile(SceneNode& node, const TextureHolder& textures);


private:
	Type mType;
	Behavors mBehavors;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;

	sf::Time mElapsedTime;
	sf::IntRect mJumpRect;
	sf::IntRect mDirectionRect;
	sf::IntRect mIdleRect;

	sf::Time mDirectionTime;
	unsigned int mCurrentDirection;
	unsigned int mPreviousDirection;
	bool isChangingDirection;
	bool isRightFace;

	unsigned int mAbilities;
	Command mFireCommand;
	bool mIsFiring;
	std::vector<Projectile*> mBullets;

	bool mIsDying; // TODO: remove it
};