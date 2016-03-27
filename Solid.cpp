#include "Solid.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


Solid::Solid(Type type, const sf::Vector2f& size)
	: mType(type)
	, mBody(size)
{
	mBody.setFillColor(sf::Color::Transparent);
	mBody.setOutlineColor(sf::Color::Red);
	mBody.setOutlineThickness(-1.f);
}

unsigned int Solid::getCategory() const
{
	return Category::Solid;
}

sf::FloatRect Solid::getBoundingRect() const
{
	return getWorldTransform().transformRect(mBody.getGlobalBounds());
}

void Solid::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//target.draw(mBody, states);
}

Type Solid::getType() const
{
	return mType;
}