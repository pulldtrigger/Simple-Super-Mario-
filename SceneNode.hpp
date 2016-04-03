#pragma once

#include "Category.hpp"
#include "Type.hpp"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector3.hpp>

#include <utility>
#include <vector>
#include <memory>
#include <set>

struct Command;
class CommandQueue;


class SceneNode : public sf::Transformable, public sf::Drawable, private sf::NonCopyable
{
public:
	using Ptr = std::unique_ptr<SceneNode>;
	using Pair = std::pair<SceneNode*, SceneNode*>;


public:
	explicit SceneNode(Category::Type category = Category::None);

	void attachChild(Ptr child);
	Ptr detachChild(const SceneNode& node);

	void update(sf::Time dt, CommandQueue& commands);

	sf::Vector2f getWorldPosition() const;
	sf::Transform getWorldTransform() const;

	void onCommand(const Command& command);
	virtual unsigned int getCategory() const;

	void removeWrecks();
	virtual sf::FloatRect getBoundingRect() const;
	virtual bool isDestroyed() const;

	virtual unsigned int getFootSenseCount() const; // it should be boolean value
	virtual void setFootSenseCount(unsigned int count);
	virtual sf::FloatRect getFootSensorBoundingRect() const;
	virtual Type getType() const;
	virtual void resolve(const sf::Vector3f& manifold, SceneNode* otherType);
	virtual void die();
	virtual unsigned int getAbilities() const;
	virtual bool isDying() const;


private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	void updateChildren(sf::Time dt, CommandQueue& commands);

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void drawChildren(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual bool isMarkedForRemoval() const;


private:
	std::vector<Ptr> mChildren;
	SceneNode* mParent;
	Category::Type mDefaultCategory;
};