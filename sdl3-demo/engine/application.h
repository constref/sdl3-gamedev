#pragma once

#include <concepts>
#include <memory>

struct FrameContext;
struct SDLState;
class GameObject;

template<typename T>
concept Application = requires(T a, SDLState &state, const FrameContext &ctx)
{
	{ a.initialize(state) } -> std::same_as<bool>;
	{ a.onStart() } -> std::same_as<void>;
	{ a.update(ctx) } -> std::same_as<void>;
	{ a.getRoot() } -> std::same_as<std::shared_ptr<GameObject> &>;
	{ a.cleanup() } -> std::same_as<void>;
};