#pragma once

#include <tuple>
#include <node.h>
#include <framestage.h>

class SystemBase
{
public:
	virtual bool hasRequiredComponents(Node &node) = 0;
	virtual void update(Node &node) = 0;
};

template<FrameStage Stage, typename... Components>
class System : public SystemBase
{
public:
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
};