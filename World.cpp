#include "World.hpp"
#include "Solid.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <iostream>

//#define Debug
namespace
{
	//contains the normal in the first two components and penetration in z
	sf::Vector3f getManifold(SceneNode::Pair& cp, sf::FloatRect overlap)
	{
		sf::Vector2f collisionNormal = cp.second->getWorldPosition() - cp.first->getWorldPosition();

		sf::Vector3f manifold;
		if (overlap.width < overlap.height)
		{
			manifold.x = (collisionNormal.x < 0) ? -1.f : 1.f;
			manifold.z = overlap.width;
		}
		else
		{
			manifold.y = (collisionNormal.y < 0) ? -1.f : 1.f;
			manifold.z = overlap.height;
		}
		return manifold;
	}
}


World::World(sf::RenderTarget& window)
	: mWindow(window)
	, mView(window.getDefaultView())
	, mTileMap()
	, mTextures()
	, mSceneGraph()
	, mCommandQueue()
	, mBodies()
	, mPlayer(nullptr)
	, mPlayerController()
{
	mView.zoom(0.5f);
	mView.setCenter(mView.getSize() / 2.f);

	loadTextures();
	buildScene();
}

void World::handleEvent(const sf::Event& event)
{
	mPlayerController.handleEvent(event, mCommandQueue);
}

void World::update(sf::Time dt)
{
	if (mPlayer && !mPlayer->isDestroyed()) // todo: this should be in player class
	{
		if (!mPlayer->isHitWall() 
			&& mView.getCenter().x < mPlayer->getWorldPosition().x 
			&& mView.getCenter().x < 500.f) // todo: make check with map boundaries
			mView.move(mPlayer->getVelocity().x * dt.asSeconds(), 0.f);
	}
	mPlayerController.handleRealtimeInput(mCommandQueue);
	//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	//{
	//	mPlayer->applyForce({ 40.f, 0.f });
	//}
	//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	//{
	//	mPlayer->applyForce({ -40.f, 0.f });
	//}
	//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	//{
	//	mPlayer->applyForce({ 0.f, -270.f }); 
	//}

	destroyEntitiesOutsideView();

	checkForCollision();

	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop());

	handleCollision();

	mSceneGraph.removeWrecks();

	mSceneGraph.update(dt, mCommandQueue);
}

void World::draw()
{
	mWindow.setView(mView);
	mWindow.draw(mTileMap);
	mWindow.draw(mSceneGraph);

#ifdef Debug
	sf::FloatRect viewBounds(mView.getCenter() - mView.getSize() / 2.f, mView.getSize());
	sf::RectangleShape debugShape({ viewBounds.width, viewBounds.height });
	debugShape.setPosition(viewBounds.left, viewBounds.top);
	debugShape.setFillColor(sf::Color::Transparent);
	debugShape.setOutlineColor(sf::Color::Cyan);
	debugShape.setOutlineThickness(-3.f);
	mWindow.draw(debugShape);
#endif // Debug
}

void World::loadTextures()
{
	mTextures.load(Textures::Player, "Media/Textures/mario_sheet.png");
}

void World::buildScene()
{
	if (!mTileMap.loadFromFile("Media/Maps/test002.tmx"))
		throw std::runtime_error("can't load level");

	for (const auto& object : mTileMap)
	{
		if (object.name == "player")
		{
			auto player(std::make_unique<Player>(Type::SmallPlayer, mTextures));
			mPlayer = player.get();
			player->setPosition(object.position);
			mSceneGraph.attachChild(std::move(player));
		}

		if (object.name == "solid")
		{
			auto solid(std::make_unique<Solid>(Type::FixedSolid, object.size));
			solid->setPosition(object.position);
			mSceneGraph.attachChild(std::move(solid));
		}
	}
}

sf::FloatRect World::getViewBounds() const
{
	return{ mView.getCenter() - mView.getSize() / 2.f, mView.getSize() };
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Player;
	command.action = derivedAction<Entity>([this](auto& entity)
	{
		if (!getViewBounds().intersects(entity.getBoundingRect()))
			entity.remove();
	});

	mCommandQueue.push(command);
}

void World::checkForCollision()
{
	mBodies.clear();

	Command command;
	command.category = Category::All;
	command.action = [this](auto& node)
	{
		if (!node.isDestroyed())
			mBodies.push_back(&node);
	};

	mCommandQueue.push(command);
}

void World::handleCollision()
{
	for (const auto& bodyA : mBodies)
	{
		bodyA->setFootSenseCount(0u);
		for (const auto& bodyB : mBodies)
		{
			if (bodyA == bodyB) continue;

			//secondary collisions with sensor boxes
			if (bodyA->getFootSensorBoundingRect().intersects(bodyB->getBoundingRect()))
			{
				unsigned int count = bodyA->getFootSenseCount();
				count++;
				bodyA->setFootSenseCount(count);
			}

			sf::FloatRect overlap;
			//primary collision between bounding boxes
			if (bodyA->getBoundingRect().intersects(bodyB->getBoundingRect(), overlap))
			{
				SceneNode::Pair pair(std::minmax(bodyA, bodyB));
				auto man = getManifold(pair, overlap);
				pair.second->resolve(man, pair.first);
				man.z = -man.z;
				pair.first->resolve(man, pair.second);
			}
		}
	}
}