#include "node.h"

#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <timer.h>
#include <logger.h>
#include <systems/system.h>

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

bool Node::isLinkedWith(SystemBase *sys)
{
	auto &stageSys = linkedSystems[static_cast<size_t>(sys->getStage())];
	if constexpr (Config::IsDebugBuild)
	{
		for (SystemBase *linkedSystem : stageSys)
		{
			if (linkedSystem == sys)
			{
				return true;
			}
		}
	}
	else
	{
		return std::ranges::find(stageSys, sys) != stageSys.end();
	}
	return false;

}

void Node::unlinkIncompatibleSystems()
{
	for (auto &stageSys : linkedSystems)
	{
		std::erase_if(stageSys, [this](SystemBase *sys) {
			if (!sys->hasRequiredComponents(*this))
			{
				sys->onUnlinked(*this);
				return true;
			}
			return false;
		});
	}
}