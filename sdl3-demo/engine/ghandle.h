#pragma once

struct GHandle
{
	size_t index;
	uint32_t generation;

	GHandle() : index(0), generation(0) {}
	GHandle(size_t idx, uint32_t gen) : index(idx), generation(gen) {}

	bool isValid() const
	{
		return generation != 0;
	}
};

