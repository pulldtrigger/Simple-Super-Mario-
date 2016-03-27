#include "TileMap.hpp"
#include "pugixml/pugixml.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <iostream>


TileMap::TileMap()
	: mVertices()
	, mTileset()
	, mObjects()
{
}

bool TileMap::loadFromFile(const std::string& filename)
{
	pugi::xml_document mapDoc;

	if (!mapDoc.load_file(filename.c_str()))
	{
		std::cerr << "Loading level \"" + filename + "\" failed.\n";
		return false;
	}

	auto mapNode = mapDoc.child("map");

	auto width = mapNode.attribute("width").as_uint();
	auto height = mapNode.attribute("height").as_uint();
	auto tileWidth = mapNode.attribute("tilewidth").as_uint();
	auto tileHeight = mapNode.attribute("tileheight").as_uint();

	mVertices.setPrimitiveType(sf::Quads);
	mVertices.resize(width * height * 4);

	auto tilesetNode = mapNode.child("tileset");

	auto firstTileID = tilesetNode.attribute("firstgid").as_uint();

	auto image = tilesetNode.child("image");

	std::string imagePath = image.attribute("source").as_string();

	auto x = imagePath.find_first_of("./");
	auto split = imagePath.substr(++x, imagePath.size());

	if (!mTileset.loadFromFile("Media" + split))
	{
		std::cerr << "can't laod texture: Media" + split + "\n";
		return false;
	}

	mTileset.setSmooth(true);

	for (auto layerNode = mapNode.child("layer"); layerNode; layerNode = layerNode.next_sibling("layer"))
	{
		auto dataNode = layerNode.child("data");
		auto tileNode = dataNode.child("tile");

		for (auto j = 0u; j < height; ++j)
		{
			for (auto i = 0u; i < width; ++i)
			{
				auto tileGID = tileNode.attribute("gid").as_uint();
				tileGID -= firstTileID;

				auto tu = tileGID % (mTileset.getSize().x / tileWidth);
				auto tv = tileGID / (mTileset.getSize().x / tileWidth);

				auto* quad = &mVertices[(i + j * width) * 4];

				quad[0].position = { static_cast<float>(i		* tileWidth), static_cast<float>(j		 * tileHeight) };
				quad[1].position = { static_cast<float>((i + 1)	* tileWidth), static_cast<float>(j		 * tileHeight) };
				quad[2].position = { static_cast<float>((i + 1)	* tileWidth), static_cast<float>((j + 1) * tileHeight) };
				quad[3].position = { static_cast<float>(i		* tileWidth), static_cast<float>((j + 1) * tileHeight) };

				//applying half pixel trick avoids artifacting when scrolling
				static const auto Fraction = 0.5f;
				quad[0].texCoords = { tu		* tileWidth + Fraction, tv		 * tileHeight + Fraction };
				quad[1].texCoords = { (tu + 1)	* tileWidth - Fraction, tv		 * tileHeight + Fraction };
				quad[2].texCoords = { (tu + 1)	* tileWidth - Fraction, (tv + 1) * tileHeight - Fraction };
				quad[3].texCoords = { tu		* tileWidth + Fraction, (tv + 1) * tileHeight - Fraction };

				tileNode = tileNode.next_sibling("tile");
			}
		}
	}

	for (auto node = mapNode.child("objectgroup"); node; node = node.next_sibling("objectgroup"))
	{
		for (auto objectNode = node.child("object"); objectNode; objectNode = objectNode.next_sibling("object"))
		{
			std::string name = objectNode.attribute("name").as_string();
			std::string type = objectNode.attribute("type").as_string();

			sf::Vector2f position;
			position.x = objectNode.attribute("x").as_float();
			position.y = objectNode.attribute("y").as_float();

			sf::Vector2f size;
			size.x = objectNode.attribute("width").as_float();
			size.y = objectNode.attribute("height").as_float();

			mObjects.emplace_back(name, type, position, size);
		}
	}
	return true;
}

sf::FloatRect TileMap::getLocalBounds() const
{
	return mVertices.getBounds();
}

sf::FloatRect TileMap::getGlobalBounds() const
{
	return getTransform().transformRect(mVertices.getBounds());
}

std::vector<TileMap::Object>::const_iterator TileMap::begin() const
{
	return mObjects.begin();
}

std::vector<TileMap::Object>::const_iterator TileMap::end() const
{
	return mObjects.end();
}

void TileMap::updateObjectsTransform()
{
	for (auto&& tile : mObjects)
	{
		tile.position = getTransform().transformPoint(tile.position);
	}
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = &mTileset;

	target.draw(mVertices, states);
}