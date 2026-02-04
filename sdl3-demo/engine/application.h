#pragma once

#include <concepts>
#include <memory>
#include <nodehandle.h>

struct SDLState;
class Services;
class VulkanRenderSystem;

template<typename T>
concept Application = requires(T a, Services &services, SDLState &state, VulkanRenderSystem &renderSys)
{
	{ a.initialize(services, state, renderSys) } -> std::same_as<bool>;
	{ a.onStart() } -> std::same_as<void>;
	{ a.getRoot() } -> std::same_as<NodeHandle>;
	{ a.cleanup() } -> std::same_as<void>;
};
