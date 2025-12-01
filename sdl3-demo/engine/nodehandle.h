#pragma once

#include <cstdint>
#include <cstddef>

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

	bool operator==(const NodeHandle &other)
	{
		return index == other.index && generation == other.generation;
	}
};

