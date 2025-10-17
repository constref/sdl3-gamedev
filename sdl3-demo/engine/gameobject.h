#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "animation.h"
#include "components/component.h"
#include "messaging/messagedispatch.h"

#include <string>
#include <unordered_map>

class GameObject
{
	glm::vec2 position;
	std::vector<std::shared_ptr<GameObject>> children;
	std::vector<Component *> components;
	MessageDispatch msgDispatch;
	bool debugHighlight;

	MessageDispatch &getMessageDispatch() { return msgDispatch; }

public:
	GameObject()
	{
		position = glm::vec2(0);
		debugHighlight = false;
	}

	~GameObject()
	{
		for (auto *comp : components)
		{
			delete comp;
		}
		components.clear();
	}

	glm::vec2 getPosition() const { return position; }
	void setPosition(const glm::vec2 position) { this->position = position; }

	auto &getChildren() { return children; }
	void addChild(std::shared_ptr<GameObject> child)
	{
		children.push_back(child);
	}

	template<typename T, typename... Args>
	T &addComponent(Args... args)
	{
		T *comp = new T(*this, args...);
		components.push_back(comp);
		return *comp;
	}

	void initializeComponents()
	{
		// register subjects
		for (Component *cmp : components)
		{
			cmp->onAttached(getMessageDispatch());
		}
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

	template<typename MessageType>
	void sendMessage(const MessageType &message)
	{
		msgDispatch.send(message);
		for (auto &child : children)
		{
			child->sendMessage(message);
		}
	}

	void notify(const FrameContext &ctx, int eventId)
	{
		for (auto &comp : components)
		{
			comp->onEvent(eventId);
		}
	}
	
	void update(const FrameContext &ctx)
	{
		for (auto &comp : components)
		{
			comp->update(ctx);
		}
	}
};