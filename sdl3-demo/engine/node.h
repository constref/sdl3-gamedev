#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <components/component.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>
#include <nodehandle.h>
#include <logger.h>

class NodeRemovalEvent;
class SystemBase;

class Node
{
protected:
	NodeHandle handle;
	NodeHandle parent;
	glm::vec2 position;
	std::vector<NodeHandle> children;
	//std::vector<Component *> components;
	std::unordered_map<size_t, Component *> components;
	std::array<std::vector<SystemBase *>, static_cast<size_t>(FrameStage::StageCount)> linkedSystems;
	bool isInitialized;
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
	int getTag() const { return tag; }
	void setTag(int tag) { this->tag = tag; }

	friend void addNodeComponent(Node &node, size_t typeId, Component &comp);
	friend void removeNodeComponent(Node &node, size_t typeId, const Component &comp);
	friend void linkSystem(Node &node, SystemBase &sys);

	auto &getStageSystems(FrameStage stage)
	{
		return linkedSystems[static_cast<size_t>(stage)];
	}

	bool isLinkedWith(SystemBase *sys);

	template<typename T>
	T *getComponent()
	{
		auto itr = components.find(Component::typeId<T>());
		if (itr != components.end())
		{
			return static_cast<T *>(itr->second);
		}
		return nullptr;
	}

	template<typename T>
	void removeComponent(const Component &comp)
	{
		auto itr = components.find(Component::typeId<T>());
		if (itr != components.end())
		{
			components.erase(itr);
			delete &comp;
		}
		else
		{
			Logger::error(this, "Couldn't remove component, address is invalid.");
		}
	}
};
