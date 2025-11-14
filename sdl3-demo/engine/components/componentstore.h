#pragma once

#include <components/component.h>
#include <systems/systemregistry.h>

class ComponentStore
{
	std::vector<Component *> components;

public:
	template<typename T, typename... Args>
	T &addComponent(Node &node, Args... args)
	{
		// create and store component
		T *comp = new T(node, args...);
		components.push_back(comp);

		return *comp;
	}

	void removeComponent(const Component &comp);
};