#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>


class Projectile final : public Entity
{
public:
	explicit Projectile(Type type, const TextureHolder& textures);

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;
	void adaptProjectileVelocity(float x);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	Type getType() const override;
	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;


private:
	Type mType;
	sf::Sprite mSprite;
	bool mIsMarkedForRemoval;
};
