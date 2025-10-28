#pragma once

#include <format>
#include <sdlstate.h>
#include <inputstate.h>
#include <framecontext.h>
#include <application.h>
#include <node.h>
#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>

template<Application AppType>
class Engine
{
	SDLState state;
	uint64_t prevTime;
	InputState inputState;
	AppType app;
	bool debugMode;
	bool running;
	constexpr static bool clampDeltaTime = true;

public:
	Engine(int logW, int logH) : state(1600, 900, logW, logH)
	{
		debugMode = false;
		running = false;
		prevTime = 0;
	}

	bool initialize()
	{
		if (state.initialize() && app.initialize(state))
		{
			return true;
		}
		return false;
	}

	void cleanup()
	{
		app.cleanup();
		state.cleanup();
	}

	void run()
	{
		const float fixedStep = 1.0f / 120.0f;
		const float dtThreshold = 1.0f / 30.0f;
		float accumulator = 0;
		prevTime = SDL_GetTicks();
		long frameCount = 0;
		double globalTime = 0;

		running = true;
		while (running)
		{
			// calculate deltaTime
			uint64_t nowTime = SDL_GetTicks();
			float actualDeltaTime = (nowTime - prevTime) / 1000.0f;
			prevTime = nowTime;

			float deltaTime = actualDeltaTime;
			if constexpr(clampDeltaTime)
			{
				// clamp actual delta time if too large due to
				// breakpoint or major slow-down
				deltaTime = std::min(deltaTime, dtThreshold);
			}

			globalTime += deltaTime;
			FrameContext ctx(state, fixedStep, globalTime, ++frameCount);
			World &world = World::get();
			Node &root = world.getNode(app.getRoot());

			SDL_Event event{ 0 };
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_EVENT_QUIT:
					{
						running = false;
						break;
					}
					case SDL_EVENT_WINDOW_RESIZED:
					{
						state.width = event.window.data1;
						state.height = event.window.data2;
						break;
					}
					case SDL_EVENT_KEY_DOWN:
					{
						// ignore repeat key-down signals while holding (prevent event spam)
						if (!event.key.repeat)
						{
							EventQueue::get().enqueue<KeyboardEvent>(InputState::get().getFocusTarget(), ComponentStage::Input, event.key.scancode, KeyboardEvent::State::down);
						}
						break;
					}
					case SDL_EVENT_KEY_UP:
					{
						EventQueue::get().enqueue<KeyboardEvent>(InputState::get().getFocusTarget(), ComponentStage::Input, event.key.scancode, KeyboardEvent::State::up);
						if (event.key.scancode == SDL_SCANCODE_F2)
						{
							debugMode = !debugMode;
						}
						else if (event.key.scancode == SDL_SCANCODE_F11)
						{
							state.fullscreen = !state.fullscreen;
							SDL_SetWindowFullscreen(state.window, state.fullscreen);
						}
						break;
					}
				}
			}


			// handle input every frame to avoid input lag
			EventQueue::get().dispatch(ComponentStage::Input);
			update(ComponentStage::Input, root, world, ctx);

			accumulator += deltaTime;
			while (accumulator >= fixedStep)
			{
				EventQueue::get().dispatch(ComponentStage::Physics);
				update(ComponentStage::Physics, root, world, ctx);

				EventQueue::get().dispatch(ComponentStage::Gameplay);
				update(ComponentStage::Gameplay, root, world, ctx);

				EventQueue::get().dispatch(ComponentStage::Animation);
				update(ComponentStage::Animation, root, world, ctx);

				accumulator -= fixedStep;
			}

			// drawing happens every single frame
			SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
			SDL_RenderClear(state.renderer);

			EventQueue::get().dispatch(ComponentStage::Render);
			update(ComponentStage::Render, root, world, ctx);

			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5, std::format("Obj: {}, EI: {}, EP: {}, EG: {}",
				World::get().getFreeCount(),
				EventQueue::get().getCount(ComponentStage::Input),
				EventQueue::get().getCount(ComponentStage::Physics),
				EventQueue::get().getCount(ComponentStage::Gameplay)
			).c_str());

			SDL_RenderPresent(state.renderer);

			EventQueue::get().dispatch(ComponentStage::PostRender);
			update(ComponentStage::PostRender, root, world, ctx);
		}
	}

	void update(ComponentStage stage, Node &obj, World &world, const FrameContext &ctx)
	{
		obj.update(stage, ctx);

		auto &children = obj.getChildren();
		for (NodeHandle &hChild : children)
		{
			Node &child = world.getNode(hChild);
			update(stage, child, world, ctx);
		}
	}
};