#pragma once

#include <array>
#include "commands.h"

class Component;

class CommandDispatch
{
	std::array<std::vector<Component *>, MAX_COMMANDS> registrations;

public:
	template<typename T>
	void registerHandler(Component *component)
	{
		registrations[T::index()].push_back(component);
	}

	template<typename T>
	void dispatch(const T &command)
	{
		auto &regs = registrations[T::index()];
		for (Component *component : regs)
		{
			//command.visit(*component);
		//	component->onCommand(command);
		}
	}
};