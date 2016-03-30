#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Player final : public Entity
{
	enum Behavors
	{
		Air,
		Ground
	};


public:
	explicit Player(Type type, const TextureHolder& textures);

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;
	void applyForce(sf::Vector2f velocity);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;
	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;

	void updateAnimation(sf::Time dt);
	void updateDirection(sf::Time dt);


private:
	Type mType;
	Behavors mBehavors;
	sf::Sprite mSprite;
	sf::RectangleShape mFootShape;
	unsigned int mFootSenseCount;
	bool mIsMarkedForRemoval;

	sf::Time mElapsedTime;
	sf::IntRect mJumpRect;
	sf::IntRect mIdleRect;
	bool mIsIdle;
};