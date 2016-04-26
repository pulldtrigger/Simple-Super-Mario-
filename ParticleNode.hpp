#pragma once

#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"
#include "Particle.hpp"
#include "Affectors.hpp"

#include <SFML/Graphics/VertexArray.hpp>

#include <deque>
#include <functional>
#include <vector>


class ParticleNode final : public SceneNode
{
	using Affector = std::function<void(Particle&, sf::Time)>;


public:
	explicit ParticleNode(Particle::Type type, const TextureHolder& textures);

	void addParticle(sf::Vector2f position);

	Particle::Type getParticleType() const;
	unsigned int getCategory() const override;

	template <typename T>
	void addAffector(const T& affector)
	{
		using namespace std::placeholders;
		mAffectors.emplace_back(std::bind(affector, _1, _2));
	}

	void emit(sf::Vector2f position);


private:
	void updateCurrent(sf::Time dt, CommandQueue& commands) override;
	void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

	void addVertex(sf::Vector2f position, sf::Vector2f texCoord, const sf::Color& color) const;
	void computeVertices() const;


private:
	std::deque<Particle> mParticles;
	const sf::Texture& mTexture;
	Particle::Type mType;

	mutable sf::VertexArray	mVertexArray;
	mutable bool mNeedsVertexUpdate;

	std::vector<Affector> mAffectors;
};