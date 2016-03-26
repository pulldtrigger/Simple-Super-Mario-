#include "Player.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Player), sf::IntRect(180, 0, 16, 16))
{
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	setScale(1.5f, 1.5f);
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}