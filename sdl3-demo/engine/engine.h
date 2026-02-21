#pragma once

#include <format>
#include <memory>
#include <array>
#include <SDL3/SDL.h>
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
#include <systems/vulkanrendersystem.h>
#include <prototypeinstancer.h>
#include <executionmode.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class Engine
{
	std::unique_ptr<Application> app;
	uint64_t prevTime;
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
	PrototypeInstancer protoInstancer;
	Services services;
	SDLState sdlState;
	vks::VulkanRenderSystem *vkRenderer;

public:
	Engine(std::unique_ptr<Application> app) : app(std::move(app)), services(world, compSys, eventQueue, inputState, protoInstancer), sdlState(SDL_GetKeyboardState(nullptr))
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

	bool initialize(int logW, int logH, int width, int height)
	{
		sdlState.width = width;
		sdlState.height = height;
		sdlState.logW = logW;
		sdlState.logH = logH;

		if constexpr (Config::IsStandaloneMode())
		{
			if (!SDL_Init(SDL_INIT_VIDEO))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
				return false;
			}

			SDL_Window *window = SDL_CreateWindow("SDL3 Demo", sdlState.width, sdlState.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
			if (!window)
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
				cleanup();
				return false;
			}
			sdlState.window = window;
			SDL_SetWindowFullscreen(window, sdlState.fullscreen);
		}

		// core system registrations
		services.compSys().registerSystem(std::make_unique<TimerSystem>(services));
		services.compSys().registerSystem(std::make_unique<InputSystem>(services));
		services.compSys().registerSystem(std::make_unique<PhysicsSystem>(services));
		services.compSys().registerSystem(std::make_unique<CollisionSystem>(services));
		services.compSys().registerSystem(std::make_unique<SpriteAnimationSystem>(services));
		auto &renderSys = services.compSys().registerSystem(std::make_unique<vks::VulkanRenderSystem>(sdlState.window, sdlState.width, sdlState.height, sdlState.logW, sdlState.logH, services));
		if (!renderSys.initialize())
		{
			return false;
		}
		vkRenderer = &renderSys;

		// initialize and start the app
		if (!app->initialize(services, sdlState))
		{
			return false;
		}

		app->start(services, sdlState);

		// TODO: This should move
		renderSys.updateTextures();

		return true;
	}

	vks::VulkanRenderSystem *getRenderer()
	{
		return vkRenderer;
	}

	Services &getServices()
	{
		return services;
	}

	void cleanup()
	{
		app->cleanup();
		services.compSys().shutdown();

		if constexpr (Config::IsStandaloneMode())
		{
			SDL_DestroyWindow(sdlState.window);
			SDL_Quit();
		}
	}

	void run()
	{
		prevTime = SDL_GetTicks();
		running = true;

#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop_arg(emIterate, this, 0, true);
#else
		while (running)
		{
			step();
		}
#endif
	}

	inline void step()
	{
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
		Node &root = world.getNode(app->getRoot());

		if (Config::IsStandaloneMode())
		{
			SDL_Event event{};
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
						sdlState.width = event.window.data1;
						sdlState.height = event.window.data2;
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
							sdlState.fullscreen = !sdlState.fullscreen;
							SDL_SetWindowFullscreen(sdlState.window, sdlState.fullscreen);
						}
						break;
					}
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

		ctx.deltaTime = deltaTime;
		// TODO: Make this better
		FrameContext::global().setStage(FrameStage::Render);
		auto &stageSystems = services.compSys().getSystemRegistry().getStageSystems(FrameContext::currentStage());
		for (auto &sys : stageSystems)
		{
			sys->beginFrame();
		}
		services.eventQueue().dispatch();
		processSystems(root, world);
		for (auto &sys : stageSystems)
		{
			sys->endFrame();
		}

		FrameContext::global().setStage(FrameStage::End);
		services.eventQueue().dispatch();
		processSystems(root, world);

		services.compSys().removeScheduled();
	}

private:
	static void emIterate(void *userData)
	{
		auto *engine = static_cast<Engine *>(userData);
		engine->step();
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
};
