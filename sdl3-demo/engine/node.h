#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <algorithm>
#include <memory>
#include <components/component.h>
#include <components/componententry.h>
#include <messaging/commanddispatcher.h>
#include <messaging/eventdispatcher.h>
#include <nodehandle.h>
#include <logger.h>
#include <config.h>

class NodeRemovalEvent;
class SystemBase;

class Node
{
protected:
	NodeHandle handle;
	NodeHandle parent;
	glm::vec2 position;
	std::vector<NodeHandle> children;
	std::vector<ComponentEntry> components;
	std::array<std::vector<SystemBase *>, static_cast<size_t>(FrameStage::StageCount)> linkedSystems;
	bool isInitialized;
	int tag;

	friend void unlinkIncompatibleSystems(Node &node);
	friend void addNodeComponent(Node &node, size_t typeId, Component &comp);
	friend void removeNodeComponent(Node &node, Component *comp);
	friend void linkSystem(Node &node, SystemBase &sys);

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

	auto &getStageSystems(FrameStage stage)
	{
		return linkedSystems[static_cast<size_t>(stage)];
	}

	bool isLinkedWith(SystemBase *sys);

	template<typename T>
	T *getComponent()
	{
		// use a linear for-loop search for debug builds
		// otherwise compiler doesn't inline function
		// call in std::find and tanks performance
		if constexpr (Config::IsDebugBuild)
		{
			for (auto &entry : components)
			{
				if (entry.id == Component::typeId<T>())
				{
					return static_cast<T *>(entry.comp);
				}
			}
		}
		else
		{
			auto itr = std::find_if(components.begin(), components.end(), [](const ComponentEntry &e) {
				return e.id == Component::typeId<T>();
			});

			if (itr != components.end())
			{
				return static_cast<T *>(itr->comp);
			}
		}
		return nullptr;
	}

	//template<typename T>
	//void removeComponent()
	//{
	//	auto itr = std::find_if(components.begin(), components.end(), [](const ComponentEntry &e) {
	//		return e.id == Component::typeId<T>();
	//	});

	//	if (itr != components.end())
	//	{
	//		components.erase(itr);
	//		unlinkIncompatibleSystems();
	//	}
	//	else
	//	{
	//		Logger::error(this, "Couldn't remove component, address is invalid.");
	//	}
	//}
};
