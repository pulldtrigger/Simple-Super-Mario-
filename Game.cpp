#include "Game.hpp"
#include "DebugText.hpp"
#include <SFML/Window/Event.hpp>


Game::Game(const std::string& title, unsigned width, unsigned height)
	: mWindow({ width, height }, title)
	, mWorld(mWindow)
	, mTitle(title)
	, mFullScreen(false)
{
	mWindow.setKeyRepeatEnabled(false);
	//mWindow.setVerticalSyncEnabled(true); // problem with text-debug
}

void Game::run()
{
	sf::Clock clock;
	auto timeSinceLastUpdate = sf::Time::Zero;
	const auto TimePerFrame = sf::seconds(1 / 60.f);

	while (mWindow.isOpen())
	{
		auto dt = clock.restart();
		timeSinceLastUpdate += dt;

		while (timeSinceLastUpdate > TimePerFrame)
		{
			timeSinceLastUpdate -= TimePerFrame;

			processEvents();
			update(TimePerFrame);
		}

		render();
	}
}

void Game::processEvents()
{
	const static auto initialSize = mWindow.getSize();

	static sf::Event event;

	while (mWindow.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			mWindow.close();

		else if (event.type == sf::Event::Resized)
		{
			auto width = static_cast<float>(event.size.width);
			auto height = static_cast<float>(event.size.height);

			auto x = initialSize.x / 2.f - width / 2.f;
			auto y = initialSize.y / 2.f - height / 2.f;

			mWindow.setView(sf::View({ x, y, width, height }));
		}

		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Escape)
				mWindow.close();

			else if (event.key.code == sf::Keyboard::F1)
			{
				mFullScreen = !mFullScreen;

				auto style = mFullScreen ? sf::Style::Fullscreen : sf::Style::Default;
				auto videoMode = mFullScreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode(initialSize.x, initialSize.y);

				mWindow.create(videoMode, mTitle, style);
			}
		}
		mWorld.handleEvent(event);
	}
}

void Game::update(sf::Time dt)
{
	mWorld.update(dt);
}

void Game::render()
{
	const static auto color = sf::Color(90, 140, 255);
	mWindow.clear(color);
	mWorld.draw();
	mWindow.display();
}