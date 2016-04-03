#pragma once

#include "Entity.hpp"

#include <SFML/Graphics/RectangleShape.hpp>


class Solid final : public Entity
{
public:
	explicit Solid(Type type, const sf::Vector2f& size);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::FloatRect getBoundingRect() const override;
	unsigned int getCategory() const override;
	Type getType() const override;


private:
	Type mType;
	sf::RectangleShape mDebugShape;
};