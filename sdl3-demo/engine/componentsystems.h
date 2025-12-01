#pragma once

#include <components/componentstore.h>
#include <systems/systemregistry.h>
#include <typeindex>
#include <queue>

void addNodeComponent(Node &node, size_t typeId, Component &comp);
void removeNodeComponent(Node &node, Component *comp);
void unlinkIncompatibleSystems();

class ComponentSystems
{
	SystemRegistry sysReg;
	ComponentStore compStore;

	std::queue<std::pair<Node *, Component *>> scheduledRemovals;

public:
	template<typename SysType>
	void registerSystem(std::unique_ptr<SysType> &&sys)
	{
		sysReg.registerSystem<SysType>(std::move(sys));
	}

	template<typename T, typename... Args>
	T &addComponent(Node &node, Args... args)
	{
		// create and store component
		T &comp = compStore.add<T>(node,  args...);
		addNodeComponent(node, Component::typeId<T>(), comp);

		for (auto &stageSystems : sysReg.getSystems())
		{
			for (auto &sys : stageSystems)
			{
				if (sys->hasRequiredComponents(node))
				{
					linkSystem(node, *sys);
				}
			}
		}
		return comp;
	}

	void removeComponent(Node &node, Component &comp);
	void removeScheduled();
	auto &getSystemRegistry() { return sysReg; }
};