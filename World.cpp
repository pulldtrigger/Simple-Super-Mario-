#include "World.hpp"
#include "Solid.hpp"
#include "Brick.hpp"
#include "ParticleNode.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <iostream>

//#define Debug
namespace
{
	sf::Vector3f getManifold(const SceneNode::Pair& node)
	{
		const auto normal = node.second->getWorldPosition() - node.first->getWorldPosition();
		sf::FloatRect overlap;

		node.first->getBoundingRect().intersects(node.second->getBoundingRect(), overlap);

		sf::Vector3f manifold;
		if (overlap.width < overlap.height)
		{
			manifold.x = (normal.x < 0) ? -1.f : 1.f;
			manifold.z = overlap.width;
		}
		else
		{
			manifold.y = (normal.y < 0) ? -1.f : 1.f;
			manifold.z = overlap.height;
		}

		return manifold;
	}
}


World::World(sf::RenderTarget& window)
	: mWindow(window)
	, mWorldView(window.getDefaultView())
	, mTileMap()
	, mTextures()
	, mSceneGraph(Category::SceneMainLayer)
	, mCommandQueue()
	, mBodies()
	, mPlayer(nullptr)
	, mPlayerController()
{
	loadTextures();
	buildScene();
}

void World::handleEvent(const sf::Event& event)
{
	mPlayerController.handleEvent(event, mCommandQueue);
}

void World::update(sf::Time dt)
{
	updateCamera();

	mPlayerController.handleRealtimeInput(mCommandQueue);

	destroyEntitiesOutsideView();

	checkForCollision();

	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop());

	mSceneGraph.removeWrecks();

	handleCollision();

	mSceneGraph.update(dt, mCommandQueue);
}

void World::draw()
{
	mWindow.setView(mWorldView);
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
	mTextures.load(Textures::Player, "Media/Textures/NES - Super Mario Bros - Mario Luigi.png");
	mTextures.load(Textures::Brick, "Media/Textures/NES - Super Mario Bros - Tileset.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::Items, "Media/Textures/NES - Super Mario Bros - Items Objects.png");
}

void World::buildScene()
{
	if (!mTileMap.loadFromFile("Media/Maps/test006.tmx"))
		throw std::runtime_error("can't load level");

	mWorldBounds.left = mWorldBounds.top = 0.f;
	mWorldBounds.width = mTileMap.getMapSize().x;
	mWorldBounds.height = mTileMap.getMapSize().y;

	mWorldView.zoom(0.5f);
	mWorldView.setCenter(mWorldView.getSize() / 2.f);

	for (const auto& object : mTileMap)
	{
		if (object.name == "player")
		{
			auto player(std::make_unique<Player>(Type::BigPlayer, mTextures));
			mPlayer = player.get();
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			player->setPosition(object.position);
			mSceneGraph.attachChild(std::move(player));
		}

		if (object.name == "solid")
		{
			auto solid(std::make_unique<Solid>(Type::Solid, object.size));
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			solid->setPosition(position);
			mSceneGraph.attachChild(std::move(solid));
		}

		if (object.name == "brick")
		{
			auto brick(std::make_unique<Brick>(Type::Brick, mTextures));
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			brick->setPosition(position);
			mSceneGraph.attachChild(std::move(brick));
		}
	}

	createParticle();
}

sf::FloatRect World::getViewBounds() const
{
	return{ mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize() };
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::All;
	command.action = derivedAction<Entity>([this](auto& entity)
	{
		if (!mWorldBounds.intersects(entity.getBoundingRect()))
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
			mBodies.emplace_back(&node);
	};

	mCommandQueue.push(command);
}

void World::handleCollision()
{
	std::set<SceneNode::Pair> collisions;

	for (const auto& bodyA : mBodies)
	{
		bodyA->setFootSenseCount(0u);
		for (const auto& bodyB : mBodies)
		{
			if (bodyA == bodyB) continue;

			//primary collision between bounding boxes
			if (bodyA->getBoundingRect().intersects(bodyB->getBoundingRect()))
			{
				collisions.insert(std::minmax(bodyA, bodyB));
			}

			//secondary collisions with sensor boxes
			if (bodyA->getFootSensorBoundingRect().intersects(bodyB->getBoundingRect()))
			{
				auto count = bodyA->getFootSenseCount();
				count++;
				bodyA->setFootSenseCount(count);
			}
		}
	}

	//resolve collision for each pair
	for (const auto& pair : collisions)
	{
		auto man = getManifold(pair);
		pair.second->resolve(man, pair.first);
		man.z = -man.z;
		pair.first->resolve(man, pair.second);
	}
}

void World::updateCamera()
{
	if (mPlayer && !mPlayer->isDestroyed())
	{
		if (mPlayer->getWorldPosition().x > mWorldView.getSize().x / 2.f
			&& mPlayer->getWorldPosition().x < mWorldBounds.width - mWorldView.getSize().x / 2.f)
			mWorldView.setCenter(mPlayer->getWorldPosition().x, mWorldView.getSize().y / 2.f);
	}
}

void World::createParticle()
{
	auto explosion(std::make_unique<ParticleNode>(Particle::Splash, mTextures));

	explosion->addAffector(ForceAffector({ 0.f, 160.f }));//gravity
	explosion->addAffector(RotateAffector(360.f));

	mSceneGraph.attachChild(std::move(explosion));
}