#pragma once

#include <messaging/dispatcher.h>
#include <nodehandle.h>

class Component;
class EventBase;
class SystemBase;

class EventPolicy
{
public:
	template<typename Type, typename RecipientType>
	static void invoke(RecipientType *recipient, NodeHandle target, const Type &obj)
	{
		recipient->onEvent(target, obj);
	}
};

class EventDispatcher : public Dispatcher<EventBase, SystemBase, EventPolicy>
{
};