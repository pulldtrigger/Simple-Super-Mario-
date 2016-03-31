#include "ParticleNode.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <iostream>

namespace
{
	static auto index = 0;
	const static auto lifetime = sf::seconds(2.5f);
	const static std::vector<sf::Vector2f> splatVelocities =
	{
		{ -30.f, -90.f },
		{  30.f, -90.f },
		{ -50.f, -70.f },
		{  50.f, -70.f },
	};
}


ParticleNode::ParticleNode(Particle::Type type, const TextureHolder& textures)
	: mParticles()
	, mTexture(textures.get(Textures::Particle))
	, mType(type)
	, mVertexArray(sf::Quads)
	, mNeedsVertexUpdate(true)
{
}

void ParticleNode::addParticle(sf::Vector2f position)
{
	Particle particle;
	particle.setPosition(position);
	particle.velocity = splatVelocities[index++ % splatVelocities.size()];
	particle.color = sf::Color(255, 255, 50);
	particle.lifetime = lifetime;

	mParticles.emplace_back(particle);
}

Particle::Type ParticleNode::getParticleType() const
{
	return mType;
}

unsigned int ParticleNode::getCategory() const
{
	return Category::ParticleSystem;
}

void ParticleNode::emit(sf::Vector2f position)
{
	for (auto i = 0; i < 4; ++i)
		addParticle(position);
}

void ParticleNode::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (mParticles.empty()) return;

	while (!mParticles.empty() && mParticles.front().lifetime <= sf::Time::Zero)
		mParticles.pop_front();

	for (auto& particle : mParticles)
	{
		particle.lifetime -= dt;
		particle.move(particle.velocity * dt.asSeconds());
		for (const auto& affector : mAffectors)
			affector(particle, dt);
	}

	mNeedsVertexUpdate = true;
}

void ParticleNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (mParticles.empty()) return;

	if (mNeedsVertexUpdate)
	{
		computeVertices();
		mNeedsVertexUpdate = false;
	}

	states.texture = &mTexture;

	target.draw(mVertexArray, states);
}

void ParticleNode::addVertex(sf::Vector2f position, sf::Vector2f texCoord, const sf::Color& color) const
{
	sf::Vertex vertex;
	vertex.position = position;
	vertex.texCoords = texCoord;
	vertex.color = color;

	mVertexArray.append(vertex);
}

void ParticleNode::computeVertices() const
{
	const static sf::Vector2f size(mTexture.getSize());
	const static sf::Vector2f half = size / 2.f;

	mVertexArray.clear();
	for (const auto& particle : mParticles)
	{
		auto color = particle.color;

		auto ratio = particle.lifetime.asSeconds() / lifetime.asSeconds();
		color.a = static_cast<sf::Uint8>(255 * std::max(ratio, 0.f));

		auto transform = particle.getTransform();
		addVertex(transform.transformPoint(-half.x, -half.y), { 0.f,	0.f	   }, color);
		addVertex(transform.transformPoint( half.x, -half.y), { size.x,	0.f    }, color);
		addVertex(transform.transformPoint( half.x,  half.y), { size.x,	size.y }, color);
		addVertex(transform.transformPoint(-half.x,  half.y), { 0.f,	size.y }, color);
	}
}