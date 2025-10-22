#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <components/component.h>
#include <messaging/messagedispatch.h>
#include <ghandle.h>

class GameObject
{
	glm::vec2 position;
	std::vector<GHandle> children;
	std::array<std::vector<Component *>, static_cast<int>(ComponentStage::SIZE)> componentStages;
	bool isInitialized;

	MessageDispatch msgDispatch;

public:
	GameObject()
	{
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
				comp->onAttached(msgDispatch);
			}
		}
		isInitialized = true;
	}

	GameObject &getObject(const GHandle &handle);

	// TODO: This will be replace with a central message queue
	template<typename MessageType>
	void sendMessage(const MessageType &message)
	{
		msgDispatch.send(message);
	}

	// TODO: This will be replace with a central message queue
	template<typename MessageType>
	void broadcastMessage(const MessageType &message)
	{
		msgDispatch.send(message);

		// propagate to children
		for (GHandle child : children)
		{
			GameObject &childObj = getObject(child);
			childObj.broadcastMessage(message);
		}
	}
};
