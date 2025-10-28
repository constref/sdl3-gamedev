#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <components/component.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>
#include <nodehandle.h>

class Node
{
	NodeHandle handle;
	glm::vec2 position;
	std::vector<NodeHandle> children;
	std::array<std::vector<Component *>, static_cast<int>(ComponentStage::SIZE)> componentStages;
	bool isInitialized;

	CommandDispatcher dataDispatcher;
	EventDispatcher eventDispatcher;

public:
	Node(NodeHandle handle) : Node()
	{
		this->handle = handle;
	}

	Node()
	{
		handle = NodeHandle(0, 0);
		position = glm::vec2(0);
		isInitialized = false;
	}

	~Node()
	{
		for (auto &stageVec : componentStages)
		{
			for (auto *comp : stageVec)
			{
				delete comp;
			}
			stageVec.clear();
		}
	}

	void update(ComponentStage stage, const FrameContext &ctx)
	{
		auto &stageVec = componentStages[static_cast<size_t>(stage)];
		for (auto &comp : stageVec)
		{
			comp->update(ctx);
		}
	}

	NodeHandle getHandle() const { return handle; }

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }

	auto &getChildren() { return children; }
	void addChild(NodeHandle childHandle)
	{
		Node &child = getObject(childHandle);
		child.initialize();

		children.push_back(childHandle);
	}

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		// create and store component
		T *comp = new T(*this, args...);

		auto &stageVec = componentStages[static_cast<size_t>(comp->getStage())];
		stageVec.push_back(comp);

		return *comp;
	}

	template<typename T>
	T *getComponent()
	{
		for (auto &stageVec : componentStages)
		{
			for (auto *comp : stageVec)
			{
				T *specific = dynamic_cast<T *>(comp);
				if (specific)
				{
					return specific;
				}
			}
		}
		return nullptr;
	}

	void initialize()
	{
		// notify all components of their syblings
		for (auto &stageVec : componentStages)
		{
			for (auto *comp : stageVec)
			{
				// allow component to initialize itself
				comp->onAttached(dataDispatcher, eventDispatcher);
			}
		}
		isInitialized = true;
	}

	Node &getObject(const NodeHandle &handle);

	template<typename EventType>
	void notify(const EventType &event)
	{
		eventDispatcher.send(event);
	}

	template<typename DPType>
	void pushData(const DPType &dp)
	{
		dataDispatcher.send(dp);
	}

	// TODO: This will be replace with a central message queue
	template<typename DPType>
	void broadcastMessage(const DPType &dp)
	{
		dataDispatcher.send(dp);

		// propagate to children
		for (NodeHandle child : children)
		{
			Node &childObj = getObject(child);
			childObj.broadcastMessage(dp);
		}
	}

	void destroySelf()
	{
	}
};
