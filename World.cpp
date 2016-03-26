#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

World::World(sf::RenderTarget& window)
	: mWindow(window)
	, mView(window.getDefaultView())
	, mTileMap()
{
	mView.zoom(0.5f);
	mView.setCenter(mView.getSize().x / 1.25f, mView.getSize().y);

	if (!mTileMap.loadFromFile("Media/Maps/test002.tmx"))
		throw std::runtime_error("can't load level");

	mTileMap.setPosition(window.getView().getSize() / 2.f);
	auto bounds(mTileMap.getLocalBounds());
	mTileMap.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
	mTileMap.updateObjectsTransform();

	// temp vars
	if (!texture.loadFromFile("Media/Textures/mario_sheet.png"))
		throw std::runtime_error("can't load texture");

	for (const auto& object : mTileMap)
	{
		if (object.name == "player")
		{
			player.setPosition(object.position);
		}
	}

	player.setTexture(texture);
	player.setTextureRect(sf::IntRect(180, 0, 16, 16));
	player.setScale(1.5f, 1.5f);
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
}

void World::draw()
{
	mWindow.setView(mView);
	mWindow.draw(mTileMap);
	mWindow.draw(player);
}