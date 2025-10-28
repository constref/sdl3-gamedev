#pragma once

#include <messaging/dispatcher.h>

class Component;
class EventBase;

class EventPolicy
{
public:
	template<typename Type, typename RecipientType>
	static void invoke(RecipientType *recipient, const Type &obj)
	{
		recipient->onEvent(obj);
	}
};

class EventDispatcher : public Dispatcher<EventBase, Component, EventPolicy>
{
};