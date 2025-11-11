#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <components/component.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>
#include <nodehandle.h>

class NodeRemovalEvent;

class Node
{
protected:
	NodeHandle handle;
	NodeHandle parent;
	glm::vec2 position;
	std::vector<NodeHandle> children;
	std::vector<Component *> components;
	bool isInitialized;
	CommandDispatcher cmdDispatcher;
	EventDispatcher eventDispatcher;

public:
	Node(NodeHandle handle);
	Node();
	virtual ~Node();

	NodeHandle getHandle() const { return handle; }
	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }
	auto &getParent() const { return parent; }
	auto &getChildren() { return children; }
	void addChild(NodeHandle childHandle);
	void removeChild(NodeHandle childHandle);
	CommandDispatcher &getCommandDispatcher() { return cmdDispatcher; }
	EventDispatcher &getEventDispatcher() { return eventDispatcher; }

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		// create and store component
		T *comp = new T(*this, args...);
		components.push_back(comp);

		return *comp;
	}

	template<typename T>
	T *getComponent()
	{
		for (auto *comp : components)
		{
			T *specific = dynamic_cast<T *>(comp);
			if (specific)
			{
				return specific;
			}
		}
		return nullptr;
	}


	void removeComponent(const Component &comp);

	void initialize()
	{
		isInitialized = true;
	}

	Node &getNode(const NodeHandle &handle);

	template<typename EventType>
	void notify(const EventType &event)
	{
		eventDispatcher.send(event);
	}

	template<typename DPType>
	void sendCommand(const DPType &dp)
	{
		cmdDispatcher.send(dp);
	}

	// TODO: This will be replace with a central message queue
	template<typename DPType>
	void broadcastMessage(const DPType &dp)
	{
		cmdDispatcher.send(dp);

		// propagate to children
		for (NodeHandle child : children)
		{
			Node &childObj = getNode(child);
			childObj.broadcastMessage(dp);
		}
	}

	void scheduleDestroy(float delay = 0);
};
