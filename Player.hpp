#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Type.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <iostream>

class Player : public Entity
{
public:
	explicit Player(Type type, const TextureHolder& textures);

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const;


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

private:
	Type mType;
	sf::Sprite mSprite;
	bool mIsMarkedForRemoval;
};