#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <components/component.h>
#include <messaging/datadispatcher.h>
#include <messaging/eventdispatcher.h>
#include <ghandle.h>

class GameObject
{
	GHandle handle;
	glm::vec2 position;
	std::vector<GHandle> children;
	std::array<std::vector<Component *>, static_cast<int>(ComponentStage::SIZE)> componentStages;
	bool isInitialized;

	DataDispatcher dataDispatcher;
	EventDispatcher eventDispatcher;

public:
	GameObject(GHandle handle) : GameObject()
	{
		this->handle = handle;
	}

	GameObject()
	{
		handle = GHandle(0, 0);
		position = glm::vec2(0);
		isInitialized = false;
	}

	~GameObject()
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

	GHandle getHandle() const { return handle; }

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }

	auto &getChildren() { return children; }
	void addChild(GHandle childHandle)
	{
		GameObject &child = getObject(childHandle);
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

	GameObject &getObject(const GHandle &handle);

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
		for (GHandle child : children)
		{
			GameObject &childObj = getObject(child);
			childObj.broadcastMessage(dp);
		}
	}
};
