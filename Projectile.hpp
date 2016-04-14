#pragma once


#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>


class Projectile final : public Entity
{
public:
	enum Type
	{
		PlayerProjectile,
		TypeCount
	};


public:
	explicit Projectile(Type type, const TextureHolder& textures);

	void adaptProjectileVelocity(float vx);


private:
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;

	sf::FloatRect getBoundingRect() const override;
	bool isMarkedForRemoval() const override;
	unsigned int getCategory() const override;

	void resolve(const sf::Vector3f& manifold, SceneNode* otherType) override;


private:
	Type mType;
	sf::Sprite mSprite;
	bool mIsMarkedForRemoval;
	sf::Time mTimeDely;
	bool mIsDying;
};
