#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <string>
#include <sstream>

#define debug DebugText::instance()

class DebugText
{
public:
	static DebugText& instance()
	{
		static DebugText d;
		return d;
	}

	DebugText()
	{
		if (!font.loadFromFile("Media/arial.ttf"))
			throw std::runtime_error("can't load fonts");

		text.setFont(font);
		text.setPosition(20.f, 0.f);
		text.setString(" ");
		text.setCharacterSize(10u);
	}

	template <class T>
	DebugText &operator<<(const T& obj)
	{
		stream << obj;
		needUpdate = true;
		return *this;
	}

	void setPosition(sf::Vector2f pos)
	{
		text.setPosition(pos);
	}

	void draw(sf::RenderTarget& target)
	{
		if (needUpdate)
		{
			text.setString(stream.str());
			stream.clear();
			stream.flush();
			stream.str({});
			needUpdate = false;
		}

		target.draw(text);
	}


private:
	sf::Font font;
	sf::Text text;
	std::stringstream stream;
	bool needUpdate = false;
};