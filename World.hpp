#pragma once

#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "TileMap.hpp"
#include "SceneNode.hpp"
#include "Player.hpp"
#include "CommandQueue.hpp"
#include "PlayerController.hpp"

#include <SFML/Graphics/View.hpp>


namespace sf
{
	class RenderTarget;
}

class World : sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& window);

	void handleEvent(const sf::Event& event);
	void update(sf::Time dt);
	void draw();


private:
	void loadTextures();
	void buildScene();

	void destroyEntitiesOutsideView();
	sf::FloatRect getViewBounds() const;

	void checkForCollision();
	void handleCollision();

	void updateCamera();
	void createParticle();


private:
	sf::RenderTarget& mWindow;
	sf::View mWorldView;
	sf::FloatRect mWorldBounds;
	TileMap mTileMap;
	TextureHolder mTextures;
	SceneNode mSceneGraph;
	CommandQueue mCommandQueue;
	std::vector<SceneNode*> mBodies;
	Player* mPlayer;
	PlayerController mPlayerController;
};
