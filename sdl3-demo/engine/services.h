#pragma once

class World;
class EventQueue;
class InputState;
class ComponentSystems;

class Services
{
	World &mworld;
	ComponentSystems &mcompSys;
	EventQueue &meventQueue;
	InputState &minputState;

public:
	Services(World &worldIn, ComponentSystems &compSysIn, EventQueue &eventQueueIn, InputState &inputStateIn) :
		mworld(worldIn), mcompSys(compSysIn), meventQueue(eventQueueIn), minputState(inputStateIn)
	{
	}

	auto &world() { return mworld; }
	auto &compSys() { return mcompSys; }
	auto &eventQueue() { return meventQueue; }
	auto &inputState() { return minputState; }
};