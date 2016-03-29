#pragma once

#include "Entity.hpp"

#include <SFML/Graphics/RectangleShape.hpp>


class Solid final : public Entity
{
public:
	explicit Solid(Type type, const sf::Vector2f& size);

	sf::FloatRect getBoundingRect() const override;
	unsigned int getCategory() const override;


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	Type getType() const override;


private:
	Type mType;
	sf::RectangleShape mDebugShape;
};