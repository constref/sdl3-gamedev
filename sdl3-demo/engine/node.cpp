#include "node.h"

#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <timer.h>
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
	tag = 0;
}

Node::~Node()
{
	for (auto *comp : components)
	{
		delete comp;
	}
	components.clear();
}

void Node::addChild(Node &child)
{
	child.parent = handle;
	children.push_back(child.handle);
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
	auto itr = std::find(components.begin(), components.end(), &comp);
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
