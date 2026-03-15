#include "engine.h"

#include "systems/enginesystem.h"
#include "systems/windowingsystem.h"

Engine::Engine(std::unique_ptr<Application> app): app(std::move(app)), services(world, compSys, eventQueue, inputState, protoInstancer), sdlState(SDL_GetKeyboardState(nullptr))
{
	debugMode = false;
	running = false;
	prevTime = 0;
	accumulator = 0;
	frameCount = 0;
	globalTime = 0;
}

Engine::~Engine()
{
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#endif
	cleanup();
}

bool Engine::initialize(int logW, int logH, int width, int height)
{
	sdlState.logW = logW;
	sdlState.logH = logH;

	SDL_Window *window = nullptr;
	if constexpr (Config::IsStandaloneMode())
	{
		auto windowSystem = std::make_unique<WindowingSystem>(services);
		windowSystem->initialize(width, height);
		window = windowSystem->window();
		services.compSys().registerSystem(std::move(windowSystem));
	}

	// core system registrations
	services.compSys().registerSystem(std::make_unique<TimerSystem>(services));
	services.compSys().registerSystem(std::make_unique<InputSystem>(services));
	services.compSys().registerSystem(std::make_unique<PhysicsSystem>(services));
	services.compSys().registerSystem(std::make_unique<CollisionSystem>(services));
	services.compSys().registerSystem(std::make_unique<SpriteAnimationSystem>(services));
	auto &renderSys = services.compSys().registerSystem(std::make_unique<d3d12rs::D3D12RenderSystem>(services, window, width, height, logW, logH));
	if (!renderSys.initialize())
	{
		return false;
	}
	d3d12Renderer = &renderSys;
	services.compSys().registerSystem(std::make_unique<EngineSystem>(services, *this));

	//auto &renderSys = services.compSys().registerSystem(std::make_unique<vks::VulkanRenderSystem>(sdlState.window, sdlState.width, sdlState.height, sdlState.logW, sdlState.logH, services));
	//if (!renderSys.initialize())
	//{
	//	return false;
	//}
	//vkRenderer = &renderSys;

	// initialize and start the app
	if (!app->initialize(services, sdlState))
	{
		return false;
	}

	app->start(services, sdlState);

	// TODO: This should move
	//renderSys.updateTextures();

	return true;
}

d3d12rs::D3D12RenderSystem *Engine::getRenderer() const
{
	return d3d12Renderer;
}

Services& Engine::getServices()
{
	return services;
}

void Engine::cleanup()
{
	app->cleanup();
	services.compSys().shutdown();
}

void Engine::run()
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

void Engine::stop()
{
	running = false;
}

void Engine::step()
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
		deltaTime = min(deltaTime, dtThreshold);
	}

	globalTime += deltaTime;

	FrameContext &ctx = FrameContext::global();
	ctx.deltaTime = deltaTime;
	ctx.globalTime = globalTime;
	ctx.frameNumber = ++frameCount;

	World &world = services.world();
	Node &root = world.getNode(app->getRoot());
	
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

void Engine::processSystems(Node& obj, World& world)
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

void Engine::emIterate(void *userData)
{
	auto *engine = static_cast<Engine *>(userData);
	engine->step();
}
