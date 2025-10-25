#pragma once

#include <format>
#include <sdlstate.h>
#include <inputstate.h>
#include <framecontext.h>
#include <application.h>
#include <gameobject.h>
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
		float accumulator = 0;
		prevTime = SDL_GetTicks();
		long frameCount = 0;

		running = true;
		while (running)
		{
			// calculate deltaTime
			uint64_t nowTime = SDL_GetTicks();
			float deltaTime = (nowTime - prevTime) / 1000.0f;
			prevTime = nowTime;

			FrameContext ctx(state, inputState, fixedStep, ++frameCount);
			World &world = World::get();
			GameObject &root = world.getObject(app.getRoot());

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
						EventQueue::get().enqueue<KeyboardEvent>(InputState::get().getFocusTarget(), event.key.scancode, KeyboardEvent::State::down);
						break;
					}
					case SDL_EVENT_KEY_UP:
					{
						EventQueue::get().enqueue<KeyboardEvent>(InputState::get().getFocusTarget(), event.key.scancode, KeyboardEvent::State::up);
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

			accumulator += deltaTime;
			if (accumulator >= fixedStep)
			{
				// Dispatch input related events to input stage component(s)
				EventQueue::get().dispatch(ComponentStage::Input);
				update(ComponentStage::Input, root, world, ctx);

				// TODO: Post-input events

				update(ComponentStage::Physics, root, world, ctx);

				// TODO: Post-physics events

				update(ComponentStage::Gameplay, root, world, ctx);
				update(ComponentStage::Animation, root, world, ctx);

				// TODO: Pre-render events

				accumulator -= fixedStep;
			}

			SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
			SDL_RenderClear(state.renderer);

			update(ComponentStage::Render, root, world, ctx);

			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5, std::format("Objects: {}, Events: {}", World::get().getFreeCount(), EventQueue::get().getCount()).c_str());

			SDL_RenderPresent(state.renderer);

			update(ComponentStage::PostRender, root, world, ctx);
		}
	}

	void update(ComponentStage stage, GameObject &obj, World &world, const FrameContext &ctx)
	{
		obj.update(stage, ctx);

		auto &children = obj.getChildren();
		for (GHandle &hChild : children)
		{
			GameObject &child = world.getObject(hChild);
			update(stage, child, world, ctx);
		}
	}
};