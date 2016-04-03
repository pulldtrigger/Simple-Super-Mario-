#pragma once


#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Goomba final : public Entity
{
	enum Behavors
	{
		Air,
		Ground,
		Dying
	};


public:
	explicit Goomba(Type type, const TextureHolder& textures);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	sf::FloatRect getFootSensorBoundingRect() const override;

	void setFootSenseCount(unsigned int count) override;
	unsigned int getFootSenseCount() const override;
	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;

	void die() override;
	bool isDying() const override;

	void updateAnimation(sf::Time dt);


private:
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
};
