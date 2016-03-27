#pragma once


#include <functional>


class SceneNode;


struct Command
{
	using Action = std::function<void(SceneNode&)>;

	Command();
	Action			action;
	unsigned int	category;
};


template
<
	typename GameObject,
	typename Function,
	typename = std::enable_if_t<std::is_base_of<SceneNode, GameObject>::value>
>
auto derivedAction(Function fn)
{
	return [=](SceneNode& node)
	{
		fn(static_cast<GameObject&>(node));
	};
}
