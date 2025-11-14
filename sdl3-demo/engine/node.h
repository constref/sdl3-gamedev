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
	std::vector<SystemBase *> linkedSystems;
	bool isInitialized;
	CommandDispatcher cmdDispatcher;
	EventDispatcher eventDispatcher;
	int tag;

public:
	Node(NodeHandle handle);
	Node();
	virtual ~Node();

	NodeHandle getHandle() const { return handle; }
	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }
	auto &getParent() const { return parent; }
	auto &getChildren() { return children; }
	void addChild(Node &child);
	void removeChild(NodeHandle childHandle);
	CommandDispatcher &getCommandDispatcher() { return cmdDispatcher; }
	EventDispatcher &getEventDispatcher() { return eventDispatcher; }
	int getTag() const { return tag; }
	void setTag(int tag) { this->tag = tag; }


	friend void addNodeComponent(Node &node, Component &comp);
	friend void removeNodeComponent(Node &node, const Component &comp);

	void removeComponent(const Component &comp);

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

	void scheduleDestroy(float delay = 0);
};
