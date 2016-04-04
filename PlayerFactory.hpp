#pragma once


#include "ResourceIdentifiers.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/NonCopyable.hpp>

class Player;
class CommandQueue;
class SceneNode;


class PlayerFactory final : private sf::NonCopyable
{
public:
	explicit PlayerFactory(const TextureHolder& textures);

	void setSceneNode(SceneNode* node);
	Player* create(sf::Vector2f position) const;
	Player* spawn() const;
	bool update(sf::Time dt) const;


private:
	mutable Player* mPlayer;
	SceneNode* mSceneNode;
	const TextureHolder& mTextures;

	mutable sf::Time mTimer;
	mutable sf::Vector2f mPosition;
	mutable bool mIsAlive;
	mutable bool mCanSpawn;
};
