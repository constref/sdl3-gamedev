#pragma once

#include <concepts>
#include <memory>
#include <nodehandle.h>

struct SDLState;

template<typename T>
concept Application = requires(T a, SDLState &state)
{
	{ a.initialize(state) } -> std::same_as<bool>;
	{ a.onStart() } -> std::same_as<void>;
	{ a.getRoot() } -> std::same_as<NodeHandle>;
	{ a.cleanup() } -> std::same_as<void>;
};
