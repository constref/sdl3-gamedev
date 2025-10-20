#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <components/component.h>
#include <messaging/messagedispatch.h>
#include <ghandle.h>

class GameObject
{
	glm::vec2 position;
	std::vector<GHandle> children;
	std::vector<Component *> components;

	MessageDispatch msgDispatch;

public:
	GameObject()
	{
		position = glm::vec2(0);
	}

	~GameObject()
	{
		for (auto *comp : components)
		{
			delete comp;
		}
		components.clear();
	}

	void update(const FrameContext &ctx)
	{
		for (auto &comp : components)
		{
			comp->update(ctx);
		}
	}

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }

	auto &getChildren() { return children; }
	void addChild(GHandle childHandle)
	{
		children.push_back(childHandle);
	}

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		// create and store component
		T *comp = new T(*this, args...);
		components.push_back(comp);

		// allow component to initialize itself
		comp->onAttached(msgDispatch);
		return *comp;
	}

	template<typename T>
	T *getComponent()
	{
		for (auto *comp : components)
		{
			T *specific = dynamic_cast<T*>(comp);
			if (specific)
			{
				return specific;
			}
		}
		return nullptr;
	}

	GameObject &getObject(const GHandle &handle);

	// TODO: This will be replace with a central message queue
	template<typename MessageType>
	void sendMessage(const MessageType &message)
	{
		msgDispatch.send(message);

		// propagate to children
		for (auto &child : children)
		{
			GameObject &childObj = getObject(child);
			childObj.sendMessage(message);
		}
	}
};
