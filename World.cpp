#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <iostream>

//#define Debug

World::World(sf::RenderTarget& window)
	: mWindow(window)
	, mView(window.getDefaultView())
	, mTileMap()
	, mTextures()
	, mSceneGraph()
{
	mView.zoom(0.5f);
	mView.setCenter(mView.getSize().x / 1.25f, mView.getSize().y);

	loadTextures();
	buildScene();
}

void World::update(sf::Time dt)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		mView.move(100.f * dt.asSeconds(), 0.f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		mView.move(-100.f * dt.asSeconds(), 0.f);
	}
	destroyEntitiesOutsideView();

	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop());

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

	mTileMap.setPosition(mWindow.getView().getSize() / 2.f);
	auto bounds(mTileMap.getLocalBounds());
	mTileMap.setOrigin(std::floor(bounds.width / 2.f), std::floor(bounds.height / 2.f));
	mTileMap.updateObjectsTransform();

	for (const auto& object : mTileMap)
	{
		if (object.name == "player")
		{
			std::unique_ptr<Player> player(std::make_unique<Player>(Type::SmallPlayer, mTextures));
			player->setPosition(object.position);
			player->setVelocity(0.f, 50.f);
			mSceneGraph.attachChild(std::move(player));
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