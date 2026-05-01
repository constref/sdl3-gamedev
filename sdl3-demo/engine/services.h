#pragma once

class World;
class EventQueue;
class InputState;
class ComponentSystems;
class PrototypeInstancer;
class AssetManager;

class Services
{
	AssetManager &m_assetManager;
	World &m_world;
	ComponentSystems &m_compSys;
	EventQueue &m_eventQueue;
	InputState &m_inputState;
	PrototypeInstancer &m_protoInstancer;

public:
	Services(AssetManager &assetManager, World &worldIn, ComponentSystems &compSysIn, EventQueue &eventQueueIn, InputState &inputStateIn, PrototypeInstancer &protoInstancer) :
		m_assetManager(assetManager), m_world(worldIn), m_compSys(compSysIn), m_eventQueue(eventQueueIn),
		m_inputState(inputStateIn), m_protoInstancer(protoInstancer)
	{
	}
	
	auto &assetManager() { return m_assetManager; }
	auto &world() { return m_world; }
	auto &compSys() { return m_compSys; }
	auto &eventQueue() { return m_eventQueue; }
	auto &inputState() { return m_inputState; }
	auto &protoInstancer() { return m_protoInstancer; }
};
