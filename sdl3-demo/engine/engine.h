#pragma once

#include <format>
#include <memory>
#include <array>
#include <sdlstate.h>
#include <inputstate.h>
#include <framecontext.h>
#include <application.h>
#include <node.h>
#include <world.h>
#include <messaging/eventqueue.h>
#include <messaging/events.h>
#include <componentsystems.h>
#include <systems/inputsystem.h>
#include <systems/spriterendersystem.h>
#include <systems/spriteanimationsystem.h>
#include <systems/physicssystem.h>
#include <systems/collisionsystem.h>
#include <systems/timersystem.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

template<Application AppType>
class Engine
{
	uint64_t prevTime;
	AppType app;
	bool debugMode;
	bool running;
	constexpr static bool clampDeltaTime = true;

	const float fixedStep = 1.0f / 120.0f;
	const float dtThreshold = 1.0f / 30.0f;
	float accumulator;
	long frameCount;
	double globalTime;

	// core services
	ComponentSystems compSys;
	EventQueue eventQueue;
	World world;
	InputState inputState;
	Services services;

public:
	Engine() : services(world, compSys, eventQueue, inputState)
	{
		debugMode = false;
		running = false;
		prevTime = 0;
		accumulator = 0;
		frameCount = 0;
		globalTime = 0;
	}
	~Engine()
	{
#ifdef __EMSCRIPTEN__
		emscripten_cancel_main_loop();
#endif
		cleanup();
	}

	bool initialize(int logW, int logH)
	{
		SDLState &state = SDLState::global();
		if (state.initialize(1600, 900, logW, logH))
		{
			// core system registrations
			services.compSys().registerSystem(std::make_unique<TimerSystem>(services));
			services.compSys().registerSystem(std::make_unique<InputSystem>(services));
			services.compSys().registerSystem(std::make_unique<PhysicsSystem>(services));
			services.compSys().registerSystem(std::make_unique<CollisionSystem>(services));
			services.compSys().registerSystem(std::make_unique<SpriteAnimationSystem>(services));
			services.compSys().registerSystem(std::make_unique<SpriteRenderSystem>(services));

			return app.initialize(services, state);
		}

		return false;
	}

	void cleanup()
	{
		app.cleanup();
		SDLState::global().cleanup();
	}

	void run()
	{
		prevTime = SDL_GetTicks();
		running = true;
		SDLState &state = SDLState::global();

#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop_arg(emIterate, this, 0, true);
#else
		while (running)
		{
			step();
		}
#endif
	}

private:
	static void emIterate(void *userData)
	{
		auto *engine = static_cast<Engine<AppType> *>(userData);
		engine->step();
	}

	inline void step()
	{
		SDLState &state = SDLState::global();
		// calculate deltaTime
		uint64_t nowTime = SDL_GetTicks();
		float actualDeltaTime = (nowTime - prevTime) / 1000.0f;
		prevTime = nowTime;

		float deltaTime = actualDeltaTime;
		if constexpr (clampDeltaTime)
		{
			// clamp actual delta time if too large due to
			// breakpoint or major slow-down
			deltaTime = std::min(deltaTime, dtThreshold);
		}

		globalTime += deltaTime;

		FrameContext &ctx = FrameContext::global();
		ctx.deltaTime = deltaTime;
		ctx.globalTime = globalTime;
		ctx.frameNumber = ++frameCount;

		World &world = services.world();
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
						services.eventQueue().enqueue<KeyDownEvent>(services.inputState().getFocusTarget(), 0, event.key.scancode);
					}
					break;
				}
				case SDL_EVENT_KEY_UP:
				{
					services.eventQueue().enqueue<KeyUpEvent>(services.inputState().getFocusTarget(), 0, event.key.scancode);
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

		FrameContext::global().setStage(FrameStage::Start);
		services.eventQueue().dispatch();
		processSystems(root, world);

		FrameContext::global().setStage(FrameStage::Input);
		services.eventQueue().dispatch();
		processSystems(root, world);

		// fixed step systems
		ctx.deltaTime = fixedStep;
		accumulator += deltaTime;
		const float accumulatorBackup = accumulator;

		FrameContext::global().setStage(FrameStage::Physics);
		services.eventQueue().dispatch();
		while (accumulator >= fixedStep)
		{
			processSystems(root, world);
			accumulator -= fixedStep;
		}

		FrameContext::global().setStage(FrameStage::Gameplay);
		services.eventQueue().dispatch();
		accumulator = accumulatorBackup;
		while (accumulator >= fixedStep)
		{
			processSystems(root, world);
			accumulator -= fixedStep;
		}

		FrameContext::global().setStage(FrameStage::Animation);
		services.eventQueue().dispatch();
		accumulator = accumulatorBackup;
		while (accumulator >= fixedStep)
		{
			processSystems(root, world);
			accumulator -= fixedStep;
		}

		// drawing happens every single frame
		ctx.deltaTime = deltaTime;
		SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		SDL_RenderClear(state.renderer);

		FrameContext::global().setStage(FrameStage::Render);
		services.eventQueue().dispatch();
		processSystems(root, world);

		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_RenderDebugText(state.renderer, 5, 5, std::format("{:.3f} N: {} E: {}",
			actualDeltaTime,
			services.world().getFreeCount(),
			services.eventQueue().getCount()
		).c_str());

		SDL_RenderPresent(state.renderer);

		FrameContext::global().setStage(FrameStage::End);
		services.eventQueue().dispatch();
		processSystems(root, world);

		services.compSys().removeScheduled();
	}

	void processSystems(Node &obj, World &world)
	{
		auto &stageSystems = obj.getStageSystems(FrameContext::currentStage());
		for (auto &sys : stageSystems)
		{
			sys->update(obj);
		}
		auto &children = obj.getChildren();
		for (NodeHandle &hChild : children)
		{
			Node &child = world.getNode(hChild);
			processSystems(child, world);
		}
	}

	void processNodes(std::unique_ptr<SystemBase> &sys, Node &obj, World &world)
	{
		sys->update(obj);

		auto &children = obj.getChildren();
		for (NodeHandle &hChild : children)
		{
			Node &child = world.getNode(hChild);
			processNodes(sys, child, world);
		}
	}
};
