#include <componentsystems.h>
#include <components/componententry.h>
#include <logger.h>

void addNodeComponent(Node &node, size_t typeId, Component &comp)
{
	auto [itr, inserted] = node.components.emplace_back(ComponentEntry({ typeId, &comp }));
	if (!inserted)
	{
		Logger::error(&node, "Tried adding an already existing component to this node.");
	}
}

void removeNodeComponent(Node &node, Component *comp)
{
	const auto &entry = std::find_if(node.components.begin(), node.components.end(), [comp](const ComponentEntry &e) {
		return e.comp == comp;
	});

	if (entry != node.components.end())
	{
		node.components.erase(entry);
	}
	else
	{
		Logger::error(&node, "Couldn't remove component, component type wasn't found in the Node.");
	}

	unlinkIncompatibleSystems(node);
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

void ComponentSystems::removeComponent(Node &node, Component &comp)
{
	scheduledRemovals.push(std::make_pair(&node, &comp));
}

void ComponentSystems::removeScheduled()
{
	for (; !scheduledRemovals.empty(); scheduledRemovals.pop())
	{
		auto &s = scheduledRemovals.front();
		removeNodeComponent(*s.first, s.second);
		compStore.remove(*s.second);
	}
}

void unlinkIncompatibleSystems(Node &node)
{
	for (auto &stageSys : node.linkedSystems)
	{
		std::erase_if(stageSys, [&node](SystemBase *sys) {
			if (!sys->hasRequiredComponents(node))
			{
				sys->onUnlinked(node);
				return true;
			}
			return false;
		});
	}
}