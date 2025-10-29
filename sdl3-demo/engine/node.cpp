#include "node.h"

#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <logger.h>

Node::Node(NodeHandle handle) : Node()
{
	this->handle = handle;
}

Node::Node()
{
	handle = NodeHandle(0, 0);
	parent = NodeHandle(0, 0);
	position = glm::vec2(0);
	isInitialized = false;
}

Node::~Node()
{
	for (auto &stageVec : componentStages)
	{
		for (auto *comp : stageVec)
		{
			comp->onDetached(dataDispatcher, eventDispatcher);
			delete comp;
		}
		stageVec.clear();
	}
}

Node &Node::getNode(const NodeHandle &handle)
{
	World &world = World::get();
	return world.getNode(handle);
}

void Node::scheduleDestroy()
{
	EventQueue::get().enqueue<NodeRemovalEvent>(getHandle(), ComponentStage::PostRender);
}

void Node::update(ComponentStage stage, const FrameContext &ctx)
{
	auto &stageVec = componentStages[static_cast<size_t>(stage)];
	for (auto &comp : stageVec)
	{
		comp->update(ctx);
	}
}

void Node::addChild(NodeHandle childHandle)
{
	Node &child = getNode(childHandle);
	child.parent = handle;
	child.initialize();
	children.push_back(childHandle);
}

void Node::removeChild(NodeHandle childHandle)
{
	auto itr = std::find(children.begin(), children.end(), childHandle);
	if (itr != children.end())
	{
		children.erase(itr);
	}
	else
	{
		Logger::warn(this, "Tried to remove non-existent child.");
	}
}

void Node::removeComponent(const Component &comp)
{
	auto &stageVec = componentStages[static_cast<size_t>(comp.getStage())];
	auto itr = std::find(stageVec.begin(), stageVec.end(), &comp);
	if (itr != stageVec.end())
	{
		stageVec.erase(itr);
		comp.onDetached(dataDispatcher, eventDispatcher);
		delete &comp;
	}
	else
	{
		Logger::error(this, "Couldn't remove component, address is invalid.");
	}
}
