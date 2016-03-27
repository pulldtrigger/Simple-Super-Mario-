#pragma once


#include "Command.hpp"

#include <SFML/System/NonCopyable.hpp>

#include <queue>


class CommandQueue final : private sf::NonCopyable
{
public:
	void push(const Command& command);
	Command pop();
	bool isEmpty() const;


private:
	std::queue<Command> mQueue;
};
