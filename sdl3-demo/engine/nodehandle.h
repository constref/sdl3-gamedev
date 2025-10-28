#pragma once

#include <cstdint>

struct NodeHandle
{
	size_t index;
	uint32_t generation;

	NodeHandle() : index(0), generation(0) {}
	NodeHandle(size_t idx, uint32_t gen) : index(idx), generation(gen) {}

	bool isValid() const
	{
		return generation != 0;
	}
};

