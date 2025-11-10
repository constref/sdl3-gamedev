#pragma once

#include <tuple>
#include <node.h>

class SystemBase
{
public:
	virtual bool hasRequiredComponents(Node &node) = 0;
	virtual void update(Node &node) = 0;
};

template<typename... Components>
class System : public SystemBase
{
public:
	using NodeComponents = std::tuple<Components...>;

	bool hasRequiredComponents(Node &node) override
	{
		return (node.getComponent<Components>() && ...);
	}
};