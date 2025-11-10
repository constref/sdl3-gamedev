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
	static void invoke(RecipientType *recipient, const Type &obj)
	{
		recipient->onEvent(obj);
	}
};

class EventPolicy2
{
public:
	template<typename Type, typename RecipientType>
	static void invoke(RecipientType *recipient, NodeHandle target, const Type &obj)
	{
		recipient->onEvent(target, obj);
	}
};

class EventDispatcher2 : public Dispatcher<EventBase, SystemBase, EventPolicy2>
{
};

class EventDispatcher : public Dispatcher<EventBase, Component, EventPolicy>
{
};