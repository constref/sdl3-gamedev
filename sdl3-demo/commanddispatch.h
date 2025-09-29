#pragma once

#include <unordered_map>
#include <vector>

class Component;

struct Command
{
	int id;
	union
	{
		int asInt;
		float asFloat;
		bool asBool;
		void *asPtr;
	} param;
};

class CommandDispatch
{
	std::unordered_map<int, std::vector<Component *>> registrations;

public:
	void registerCommand(int commandId, Component *component)
	{
		registrations[commandId].push_back(component);
	}

	void dispatch(const Command &command)
	{
		auto &regs = registrations[command.id];
		for (Component *component : regs)
		{
			component->onCommand(command);
		}
	}
};