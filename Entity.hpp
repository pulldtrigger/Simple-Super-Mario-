#pragma once

#include "SceneNode.hpp"


class Entity : public SceneNode
{
public:
	explicit Entity(int hitpoints = 1);

	void setVelocity(float vx, float vy);
	void setVelocity(sf::Vector2f velocity);

	void destroy();
	virtual void remove();

	bool isDestroyed() const override;


protected:
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	void accelerate(sf::Vector2f velocity);


private:
	sf::Vector2f			mVelocity;
	int						mHitpoints;
};
