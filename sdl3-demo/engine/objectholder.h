#pragma once

#include <cstdint>

template<typename T>
struct ObjectHolder
{
	bool free;
	uint32_t generation;
	T object;

	ObjectHolder() : free(true), generation(0), object() {}
};
