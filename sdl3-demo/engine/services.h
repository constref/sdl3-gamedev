#pragma once

class World;
class EventQueue;
class InputState;
class ComponentSystems;
class PrototypeInstancer;

class Services
{
	World &mworld;
	ComponentSystems &mcompSys;
	EventQueue &meventQueue;
	InputState &minputState;
	PrototypeInstancer &mprotoInstancer;

public:
	Services(World &worldIn, ComponentSystems &compSysIn, EventQueue &eventQueueIn, InputState &inputStateIn, PrototypeInstancer &protoInstancer) :
		mworld(worldIn), mcompSys(compSysIn), meventQueue(eventQueueIn), minputState(inputStateIn), mprotoInstancer(protoInstancer)
	{
	}

	auto &world() { return mworld; }
	auto &compSys() { return mcompSys; }
	auto &eventQueue() { return meventQueue; }
	auto &inputState() { return minputState; }
	auto &protoInstancer() { return mprotoInstancer; }
};