#pragma once

#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "TileMap.hpp"
#include "SceneNode.hpp"
#include "Player.hpp"
#include "Tile.hpp"
#include "CommandQueue.hpp"
#include "PlayerController.hpp"

#include <SFML/Graphics/View.hpp>


namespace sf
{
	class RenderWindow;
}

class World : sf::NonCopyable
{
public:
	explicit World(sf::RenderWindow& window);

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

	void addPlayer(sf::Vector2f position);
	void addGoomba(sf::Vector2f position);
	void addBrick(sf::Vector2f position);
	void addBox(sf::Vector2f position, Tile::Item item, unsigned int count = 0);
	void addBlock(sf::Vector2f position, sf::Vector2f size);


private:
	sf::RenderWindow& mWindow;
	sf::View mWorldView;
	sf::FloatRect mWorldBounds;
	TileMap mTileMap;
	TextureHolder mTextures;
	SceneNode mSceneGraph;
	CommandQueue mCommandQueue;
	std::vector<SceneNode*> mBodies;
	std::vector<Player*> mPlayer;
	PlayerController mPlayerController;
};
