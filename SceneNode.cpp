#include "SceneNode.hpp"
#include "Command.hpp"

#include <cassert>


SceneNode::SceneNode(Category::Type category)
	: mChildren()
	, mParent(nullptr)
	, mDefaultCategory(category)
{
}

void SceneNode::attachChild(Ptr child)
{
	child->mParent = this;
	mChildren.emplace_back(std::move(child));
}

SceneNode::Ptr SceneNode::detachChild(const SceneNode& node)
{
	auto found = std::find_if(mChildren.begin(), mChildren.end(), 
		[&](auto& p)
	{ 
		return p.get() == &node;
	});

	assert(found != mChildren.end());

	auto result = std::move(*found);
	result->mParent = nullptr;
	mChildren.erase(found);
	return result;
}

void SceneNode::update(sf::Time dt, CommandQueue& commands)
{
	updateCurrent(dt, commands);
	updateChildren(dt, commands);
}

void SceneNode::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// do nothing
}

void SceneNode::updateChildren(sf::Time dt, CommandQueue& commands)
{
	for (const auto& child : mChildren)
		child->update(dt, commands);
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const
{

	states.transform *= getTransform();

	drawCurrent(target, states);
	drawChildren(target, states);
}

void SceneNode::drawCurrent(sf::RenderTarget&, sf::RenderStates) const
{
	// Do nothing by default
}

void SceneNode::drawChildren(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto& child : mChildren)
		child->draw(target, states);
}

sf::Vector2f SceneNode::getWorldPosition() const
{
	return getWorldTransform() * sf::Vector2f();
}

sf::Transform SceneNode::getWorldTransform() const
{
	auto transform = sf::Transform::Identity;

	for (const auto* node = this; node != nullptr; node = node->mParent)
		transform = node->getTransform() * transform;

	return transform;
}

void SceneNode::onCommand(const Command& command)
{
	// Command current node, if category matches
	if (command.category & getCategory())
		command.action(*this);

	// Command children
	for (const auto& child : mChildren)
		child->onCommand(command);
}

unsigned int SceneNode::getCategory() const
{
	return mDefaultCategory;
}

void SceneNode::removeWrecks()
{
	// Remove all children which request so
	mChildren.erase(
		std::remove_if(mChildren.begin(), mChildren.end(),
			std::mem_fn(&SceneNode::isMarkedForRemoval)),
		mChildren.end());

	// Call function recursively for all remaining children
	std::for_each(mChildren.begin(), mChildren.end(), std::mem_fn(&SceneNode::removeWrecks));
}

sf::FloatRect SceneNode::getBoundingRect() const
{
	return sf::FloatRect();
}

bool SceneNode::isDestroyed() const
{
	// By default, scene node needn't be removed
	return false;
}

bool SceneNode::isMarkedForRemoval() const
{
	// By default, remove node if entity is destroyed
	return isDestroyed();
}

unsigned int SceneNode::getFootSenseCount() const
{
	return 0;
}

void SceneNode::setFootSenseCount(unsigned int count)
{
}

sf::FloatRect SceneNode::getFootSensorBoundingRect() const
{
	return{};
}

Type SceneNode::getType() const
{
	return Type::None;
}

void SceneNode::resolve(const sf::Vector3f& manifold, SceneNode* otherType)
{
}

void SceneNode::die()
{
}

unsigned int SceneNode::getAbilities() const
{ 
	return 0;
}

bool SceneNode::isDying() const
{
	return false;
}