#include "Game.hpp"

#include <SFML/Window/Event.hpp>


Game::Game(const std::string& title, unsigned width, unsigned height)
	: mWindow(sf::VideoMode(width, height), title)
	, mWorld(mWindow)
	, mTitle(title)
	, mFullScreen(false)
{
	//auto widthi = 1024.f;
	//auto heighti = 512.f;
	//auto initialSize = mWindow.getSize();
	//auto x = initialSize.x / 2.f - widthi / 2.f;
	//auto y = initialSize.y / 2.f - heighti / 2.f;
	//mWindow.setSize(sf::Vector2u{ 1024, 512 });
	//mWindow.setView(sf::View({ x, y, widthi, heighti }));
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
	const static auto videoMode = sf::VideoMode(initialSize.x, initialSize.y);

	sf::Event event;

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

				mWindow.create(videoMode, mTitle, style);
			}
		}
	}
}

void Game::update(sf::Time dt)
{
	mWorld.update(dt);
}

void Game::render()
{
	mWindow.clear();
	mWorld.draw();
	mWindow.display();
}