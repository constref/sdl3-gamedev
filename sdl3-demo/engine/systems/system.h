#pragma once

#include <tuple>
#include <node.h>
#include <framestage.h>
#include <services.h>
#include <messaging/eventqueue.h>

class SystemBase
{
	FrameStage frameStage;
public:
	SystemBase(FrameStage frameStage) { this->frameStage = frameStage; }
	virtual bool hasRequiredComponents(Node &node) = 0;
	virtual void update(Node &node) = 0;
	FrameStage getStage() { return frameStage; }
};

template<FrameStage Stage, typename... Components>
class System : public SystemBase
{
protected:
	Services &services;

public:
	System(Services &services) : SystemBase(stage()), services(services) {}
	virtual ~System() {}
		
	using NodeComponents = std::tuple<Components...>;

	constexpr static FrameStage stage() { return Stage; }

	bool hasRequiredComponents(Node &node) override
	{
		return (node.getComponent<Components>() && ...);
	}

	auto getRequiredComponents(Node &node)
	{
		return std::make_tuple(node.getComponent<Components>()...);
	}

	void scheduleDestroy(NodeHandle handle, float delay)
	{
		services.eventQueue().enqueue<NodeRemovalEvent>(handle, delay);
	}
};