#pragma once

#include <limits>

class ResourceId
{
public:
	enum class Type
	{
		undefined,
		texture,
		animation
	};
private:
	unsigned int resourceIndex = UINT_MAX;
	Type resourceType = Type::undefined;

public:

	ResourceId() = default;
	ResourceId(unsigned int index, Type type) : resourceIndex(index), resourceType(type) {}
	auto index() const { return resourceIndex; }
	auto type() const { return resourceType; }
	bool isValid() const { return index() != UINT_MAX && type() != Type::undefined; }

	static ResourceId invalid() { return ResourceId{}; }
};
