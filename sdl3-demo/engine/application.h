#pragma once

#include <concepts>
#include <memory>
#include <nodehandle.h>

struct SDLState;
class SystemRegistry;

template<typename T>
concept Application = requires(T a, SystemRegistry &sysReg, SDLState &state)
{
	{ a.initialize(sysReg, state) } -> std::same_as<bool>;
	{ a.onStart() } -> std::same_as<void>;
	{ a.getRoot() } -> std::same_as<NodeHandle>;
	{ a.cleanup() } -> std::same_as<void>;
};
