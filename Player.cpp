#include "Player.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


Player::Player(Type type, const TextureHolder& textures)
	: mType(type)
	, mSprite(textures.get(Textures::Player), sf::IntRect(180, 0, 16, 16))
	, mIsMarkedForRemoval(false)
{
	auto bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	setScale(1.5f, 1.5f);
}

unsigned int Player::getCategory() const
{
	return Category::Player;
}

bool Player::isMarkedForRemoval() const
{
	return mIsMarkedForRemoval;
}

sf::FloatRect Player::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Player::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if(isDestroyed())
	{
		mIsMarkedForRemoval = true;
	}

	Entity::updateCurrent(dt, commands);
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}