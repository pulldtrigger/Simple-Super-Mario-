#include "PlayerController.hpp"
#include "CommandQueue.hpp"
#include "Player.hpp"


PlayerController::PlayerController()
{
	mKeyBinding.emplace(sf::Keyboard::Left, MoveLeft);
	mKeyBinding.emplace(sf::Keyboard::Right, MoveRight);
	mKeyBinding.emplace(sf::Keyboard::Up, Jumping);

	initializeActions();

	for (auto& pair : mActionBinding)
		pair.second.category = Category::Player;
}

void PlayerController::handleEvent(const sf::Event& event, CommandQueue& commands)
{
	if (event.type == sf::Event::KeyPressed)
	{
		auto found(mKeyBinding.find(event.key.code));
		if (found != mKeyBinding.end() && !isRealtimeAction(found->second))
			commands.push(mActionBinding[found->second]);
	}
}

void PlayerController::handleRealtimeInput(CommandQueue& commands)
{
	for (const auto& pair : mKeyBinding)
	{
		if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
			commands.push(mActionBinding[pair.second]);
	}
}

void PlayerController::initializeActions()
{
	using namespace std::placeholders;

	mActionBinding[MoveLeft].action = derivedAction<Player>(std::bind(&Player::applyForce, _1, sf::Vector2f(-40.f, 0.f)));
	mActionBinding[MoveRight].action = derivedAction<Player>(std::bind(&Player::applyForce, _1, sf::Vector2f(40.f, 0.f)));
	mActionBinding[Jumping].action = derivedAction<Player>(std::bind(&Player::applyForce, _1, sf::Vector2f(0.f, -470.f)));
}

bool PlayerController::isRealtimeAction(Action action)
{
	switch (action)
	{
	case MoveLeft:
	case MoveRight:return true;
	case Jumping:
	default:return false;
	}
}