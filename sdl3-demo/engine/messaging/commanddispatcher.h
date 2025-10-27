#pragma once

#include <array>
#include <vector>
#include <messaging/dispatcher.h>

class Component;
class CommandBase;

class CommandPolicy
{
public:
	template<typename Type, typename RecipientType>
	static void invoke(RecipientType *recipient, const Type &obj)
	{
		recipient->onCommand(obj);
	}
};

class CommandDispatcher : public Dispatcher<CommandBase, Component, CommandPolicy>
{
};
