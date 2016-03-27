#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <string>
#include <vector>


class TileMap final : public sf::Drawable, public sf::Transformable, private sf::NonCopyable
{
	struct Object
	{
		explicit Object(const std::string& name,
			const std::string& type,
			const sf::Vector2f& position,
			const sf::Vector2f& size)
			: name(name)
			, type(type)
			, position(position)
			, size(size)
		{}

		std::string name;
		std::string type;
		sf::Vector2f position;
		sf::Vector2f size;
	};


public:
	TileMap();

	bool loadFromFile(const std::string& filename);

	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;
	
	std::vector<Object>::const_iterator begin() const;
	std::vector<Object>::const_iterator end() const;

	void updateObjectsTransform();


private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;


private:
	sf::VertexArray mVertices;
	sf::Texture mTileset;
	std::vector<Object> mObjects;
};