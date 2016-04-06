#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <string>
#include <vector>


class TileMap final : public sf::Drawable, private sf::NonCopyable
{
	struct Object
	{
		explicit Object(const std::string& name,
			const std::string& type,
			const sf::Vector2f& position,
			const sf::Vector2f& size,
			unsigned int count)
			: name(name)
			, type(type)
			, position(position)
			, size(size)
			, count(count)
		{}

		std::string name;
		std::string type;
		sf::Vector2f position;
		sf::Vector2f size;
		unsigned int count;
	};


public:
	TileMap();

	bool loadFromFile(const std::string& filename);
	
	std::vector<Object>::const_iterator begin() const;
	std::vector<Object>::const_iterator end() const;

	sf::Vector2f getMapSize() const;


private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;


private:
	sf::VertexArray mVertices;
	sf::Texture mTileset;
	std::vector<Object> mObjects;
	sf::Vector2f mMapSize;
};