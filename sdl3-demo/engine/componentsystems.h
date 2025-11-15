#pragma once

#include <components/componentstore.h>
#include <systems/systemregistry.h>

void addNodeComponent(Node &node, Component &comp);
void removeNodeComponent(Node &node, const Component &comp);

class ComponentSystems
{
	SystemRegistry sysReg;
	ComponentStore compStore;

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
		T &comp = compStore.addComponent<T>(node, args...);
		addNodeComponent(node, comp);

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

	auto &getSystemRegistry() { return sysReg; }
};