#pragma once

#include <components/component.h>
#include <systems/systemregistry.h>

class ComponentStore
{
	std::vector<Component *> components;
	std::byte *buffer;
	size_t writeHead;

public:
	ComponentStore()
	{
		buffer = new std::byte[1024 * 1024];
		writeHead = 0;
	}

	template<typename T, typename... Args>
	T &add(Node &node, Args... args)
	{
		// create and store component
		T *comp = new (buffer + writeHead) T(node, args...);
		writeHead += sizeof(T);
		components.push_back(comp);

		return *comp;
	}

	void remove(const Component &comp);
};