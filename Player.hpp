#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "Type.hpp"
#include <SFML/Graphics/Sprite.hpp>


class Player : public Entity
{
public:
	explicit Player(Type type, const TextureHolder& textures);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;


private:
	Type mType;
	sf::Sprite mSprite;
};