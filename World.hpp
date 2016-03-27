#pragma once

#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "TileMap.hpp"
#include "SceneNode.hpp"
#include "Player.hpp"
#include "CommandQueue.hpp"

#include <SFML/Graphics/View.hpp>


namespace sf
{
	class RenderTarget;
}

class World : sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& window);

	void update(sf::Time dt);
	void draw();


private:
	void loadTextures();
	void buildScene();

	void destroyEntitiesOutsideView();
	sf::FloatRect getViewBounds() const;

	void checkForCollision();
	void handleCollision();


private:
	sf::RenderTarget& mWindow;
	sf::View mView;
	TileMap mTileMap;
	TextureHolder mTextures;
	SceneNode mSceneGraph;
	CommandQueue mCommandQueue;
	std::vector<SceneNode*> mBodies;
	Player* mPlayer;
};
