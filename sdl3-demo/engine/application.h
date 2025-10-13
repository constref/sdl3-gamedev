#pragma once

#include <concepts>
#include <memory>

struct FrameContext;
struct SDLState;

template<typename T>
concept Application = requires(T a, SDLState &state, const FrameContext &ctx)
{
	{ a.initialize(state) } -> std::same_as<bool>;
	{ a.onStart() } -> std::same_as<void>;
	{ a.update(std::declval<const FrameContext &>()) } -> std::same_as<void>;
	{ a.cleanup() } -> std::same_as<void>;
};