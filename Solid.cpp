#include "Solid.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
//#define Debug

Solid::Solid(Type type, const sf::Vector2f& size)
	: mType(type)
	, mDebugShape(size)
{
	auto bounds = mDebugShape.getLocalBounds();
	mDebugShape.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	mDebugShape.setFillColor(sf::Color::Transparent);
	mDebugShape.setOutlineColor(sf::Color::Red);
	mDebugShape.setOutlineThickness(-0.5f);
}

unsigned int Solid::getCategory() const
{
	return Category::Solid;
}

sf::FloatRect Solid::getBoundingRect() const
{
	return getWorldTransform().transformRect(mDebugShape.getGlobalBounds());
}

void Solid::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
#ifdef Debug
	target.draw(mDebugShape, states);
#endif // Debug
}

Type Solid::getType() const
{
	return mType;
}