#include <componentsystems.h>
#include <logger.h>

void addNodeComponent(Node &node, Component &comp)
{
	node.components.push_back(&comp);
}

void removeNodeComponent(Node &node, const Component &comp)
{
	auto itr = std::find(node.components.begin(), node.components.end(), &comp);
	if (itr != node.components.end())
	{
		node.components.erase(itr);
		delete &comp;
	}
	else
	{
		Logger::error(&node, "Couldn't remove component, address is invalid.");
	}
}

void linkSystem(Node &node, SystemBase &sys)
{
	auto &stageSystems = node.linkedSystems[static_cast<size_t>(sys.getStage())];
	if (std::find(stageSystems.begin(), stageSystems.end(), &sys) == stageSystems.end())
	{
		stageSystems.push_back(&sys);
		sys.onLinked(node);
	}
}