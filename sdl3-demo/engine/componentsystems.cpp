#include <componentsystems.h>
#include <logger.h>

void addNodeComponent(Node &node, size_t typeId, Component &comp)
{
	auto [itr, inserted] = node.components.insert({ typeId, &comp });
	if (!inserted)
	{
		Logger::error(&node, "Tried adding an already existing component to this node.");
	}
}

void removeNodeComponent(Node &node, size_t typeId, const Component &comp)
{
	auto itr = node.components.find(typeId);
	if (itr != node.components.end() && itr->second == &comp)
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