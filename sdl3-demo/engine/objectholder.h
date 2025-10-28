#pragma once

#include <cstdint>
#include <optional>

template<typename T>
struct ObjectHolder
{
	bool free;
	uint32_t generation;
	std::optional<T> object;

	ObjectHolder() : free(true), generation(0) {}
};
