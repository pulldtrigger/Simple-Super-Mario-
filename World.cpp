#include "World.hpp"
#include "ParticleNode.hpp"
#include "Enemy.hpp"
#include "DebugText.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <iostream>

//#define Debug
namespace
{
	bool isTile = false;
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


World::World(sf::RenderWindow& window)
	: mWindow(window)
	, mWorldView(window.getDefaultView())
	, mTileMap()
	, mTextures()
	, mSceneGraph()
	, mSceneLayers()
	, mCommandQueue()
	, mBodies()
	, mPlayer()
	, mPlayerController()
{
	loadTextures();
	buildScene();
}

void World::handleEvent(const sf::Event& event)
{
	mPlayerController.handleEvent(event, mCommandQueue);
	switch (event.type)
	{
	case sf::Event::MouseButtonPressed:
		{
			auto position = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
			switch (event.mouseButton.button)
			{
			case sf::Mouse::Left:
				//addPlayer(position);
			//{
			//	auto item(std::make_unique<Item>(Type::MoveableCoin, mTextures));
			//	item->setPosition(position + sf::Vector2f(0.f, -10.f));
			//	item->setVelocity(0.f, -475.f);
			//	mSceneLayers[Back]->attachChild(std::move(item));
			//}
			{
				auto item(std::make_unique<Item>(Item::TransformMushroom, mTextures));
				item->setPosition(position);
				item->setVelocity(40.f, -40.f);
				mSceneLayers[Back]->attachChild(std::move(item));
			}
				break;
			case sf::Mouse::Right:
				if (isTile)
					addBrick(position);
				else
					addGoomba(position);
				break;
			default: break;
			}
			break;
		}
	case sf::Event::KeyPressed:
		switch (event.key.code)
		{
		case sf::Keyboard::Num1:
			if (!mPlayer.empty())
				mPlayer.back()->applyTransformation();
			break;
		case sf::Keyboard::Num2:
			if (!mPlayer.empty())
				mPlayer.back()->applyTransformation(Player::SmallPlayer);
			break;
		case sf::Keyboard::Num3:
			if (!mPlayer.empty())
				mPlayer.back()->applyFireable();
			break;
		case sf::Keyboard::Num4:
			if (!mPlayer.empty())
				mPlayer.back()->applyFireable(Player::SmallPlayer);
			break;
		case sf::Keyboard::B:
			if(isTile)
				isTile = !isTile;
			else
				isTile = !isTile;
			break;
		default:break;
		}
		break;
	default: break;

	}
}

void World::update(sf::Time dt)
{
	mPlayer.erase( // no more sorrow
		std::remove_if(mPlayer.begin(), mPlayer.end(), 
			std::mem_fn(&Projectile::isDestroyed)), 
		mPlayer.end());

	mPlayerController.handleRealtimeInput(mCommandQueue);

	destroyEntitiesOutsideView();

	checkForCollision();

	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop());

	mSceneGraph.removeWrecks();

	handleCollision();

	if (!mPlayer.empty())
	{
		if (mPlayer.back()->paused())
		{
			mPlayer.back()->update(dt, mCommandQueue);
			return;
		}
	}

	updateCamera();

	mSceneGraph.update(dt, mCommandQueue);

	debug.setPosition(mWorldView.getCenter() - sf::Vector2f(190.f, 100.f));
}

void World::draw()
{
	mWindow.setView(mWorldView);
	debug.draw(mWindow);
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
	mTextures.load(Textures::Tile, "Media/Textures/NES - Super Mario Bros - Tileset.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::Items, "Media/Textures/NES - Super Mario Bros - Items Objects.png");
	mTextures.load(Textures::Enemies, "Media/Textures/NES - Super Mario Bros - Enemies.png");
}

void World::buildScene()
{
	for (auto i = 0u; i < LayerCount; ++i)
	{
		auto category = (i == Front) ? Category::FrontLayer : Category::BackLayer;

		auto layer(std::make_unique<SceneNode>(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

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
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			addPlayer(position);
		}

		if (object.name == "block")
		{
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			addBlock(position, object.size);
		}

		if (object.name == "brick")
		{
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			addBrick(position);
		}

		if (object.name == "box")
		{
			if (object.type == "coin")
			{
				sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
				addBox(position, Tile::SoloCoinBox);
			}

			if (object.type == "coins")
			{
				sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
				addBox(position, Tile::CoinsBox, object.count);
			}

			if (object.type == "transform")
			{
				sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
				addBox(position, Tile::TransformBox);
			}
		}

		if (object.name == "goomba")
		{
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y };
			addGoomba(position);
		}

		if (object.name == "static_coin")
		{
			sf::Vector2f position = { object.position.x + object.size.x / 2.f, object.position.y + object.size.y / 2.f };
			addItem(Item::StaticCoin, position);
		}
	}

	createParticle();
}

void World::addPlayer(sf::Vector2f position)
{
	if (mPlayer.empty())
	{
		auto player(std::make_unique<Player>(Player::SmallPlayer, mTextures));
		mPlayer.emplace_back(player.get());
		mPlayer.back()->setPosition(position);
		mSceneLayers[Front]->attachChild(std::move(player));
	}
}

void World::addGoomba(sf::Vector2f position)
{
	auto goomba(std::make_unique<Enemy>(Enemy::Goomba, mTextures));
	goomba->setPosition(position);
	goomba->setVelocity(-40.f, 0.f);
	mSceneLayers[Front]->attachChild(std::move(goomba));
}

void World::addBrick(sf::Vector2f position)
{
	auto brick(std::make_unique<Tile>(Tile::Brick, mTextures));
	brick->setPosition(position);
	mSceneLayers[Back]->attachChild(std::move(brick));
}

void World::addBlock(sf::Vector2f position, sf::Vector2f size)
{
	auto block(std::make_unique<Tile>(Tile::Block, mTextures, size));
	block->setPosition(position);
	mSceneLayers[Back]->attachChild(std::move(block));
}

void World::addBox(sf::Vector2f position, Tile::Type type, unsigned int count)
{
	auto box(std::make_unique<Tile>(type, mTextures));
	box->setPosition(position);
	box->setCoinsCount(count);
	mSceneLayers[Front]->attachChild(std::move(box));
}

void World::addItem(Item::Type type, sf::Vector2f position)
{
	auto item(std::make_unique<Item>(type, mTextures));
	item->setPosition(position);
	mSceneLayers[Back]->attachChild(std::move(item));
}

sf::FloatRect World::getViewBounds() const
{
	return{ mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize() };
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::OutOfWorld;
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
	if (!mPlayer.empty())
	{
		if (mPlayer.back()->getWorldPosition().x > mWorldView.getSize().x / 2.f
			&& mPlayer.back()->getWorldPosition().x < mWorldBounds.width - mWorldView.getSize().x / 2.f)
			mWorldView.setCenter(mPlayer.back()->getWorldPosition().x, mWorldView.getSize().y / 2.f);
	}
}

void World::createParticle()
{
	auto explosion(std::make_unique<ParticleNode>(Particle::Splash, mTextures));

	explosion->addAffector(ForceAffector({ 0.f, 160.f }));//gravity
	explosion->addAffector(RotateAffector(360.f));

	mSceneLayers[Back]->attachChild(std::move(explosion));
}