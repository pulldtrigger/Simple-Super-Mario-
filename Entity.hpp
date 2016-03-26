#pragma once

#include "SceneNode.hpp"


class Entity : public SceneNode
{
public:
	void setVelocity(const sf::Vector2f& velocity);
	void setVelocity(float vx, float vy);
	sf::Vector2f getVelocity() const;


private:
	void updateCurrent(sf::Time dt) override;


private:
	sf::Vector2f mVelocity;
};
