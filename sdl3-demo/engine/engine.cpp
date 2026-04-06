#include "engine.h"

#include "systems/enginesystem.h"
#include "systems/windowingsystem.h"

Engine::Engine(std::unique_ptr<Application> app) : m_app(std::move(app)),
                                                   m_services(world, compSys, eventQueue, inputState, protoInstancer),
                                                   sdlState(SDL_GetKeyboardState(nullptr))
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
        auto windowSystem = std::make_unique<WindowingSystem>(m_services);
        windowSystem->initialize(width, height);
        window = windowSystem->window();
        m_services.compSys().registerSystem(std::move(windowSystem));
    }

    // core system registrations
    m_services.compSys().registerSystem(std::make_unique<TimerSystem>(m_services));
    m_services.compSys().registerSystem(std::make_unique<InputSystem>(m_services));
    m_services.compSys().registerSystem(std::make_unique<PhysicsSystem>(m_services));
    m_services.compSys().registerSystem(std::make_unique<CollisionSystem>(m_services));
    m_services.compSys().registerSystem(std::make_unique<SpriteAnimationSystem>(m_services));
    
    // D3D 12
    // auto &renderSys = m_services.compSys().registerSystem(
    //     std::make_unique<d3d12rs::D3D12RenderSystem>(m_services, window, width, height, logW, logH));
    // if (!renderSys.initialize())
    // {
    //     return false;
    // }
    // renderer = &renderSys;
    // m_services.compSys().registerSystem(std::make_unique<EngineSystem>(m_services, *this));

    auto &renderSys = m_services.compSys().registerSystem(std::make_unique<vks::VulkanRenderSystem>(window, width, height, logW, logH, m_services));
    if (!renderSys.initialize())
    {
    	return false;
    }
    renderer = &renderSys;

    // initialize and start the app
    if (!m_app->initialize(m_services, sdlState))
    {
        return false;
    }

    m_app->start(m_services, sdlState);

    // TODO: This should move
    renderer->updateGPUTextures();

    return true;
}

Renderer *Engine::getRenderer() const
{
    return renderer;
}

Services& Engine::services()
{
    return m_services;
}

void Engine::cleanup()
{
    m_app->cleanup();
    m_services.compSys().shutdown();
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

    World &world = m_services.world();
    Node &root = world.getNode(m_app->getRoot());

    FrameContext::global().setStage(FrameStage::Start);
    m_services.eventQueue().dispatch();
    processSystems(root, world);

    FrameContext::global().setStage(FrameStage::Input);
    m_services.eventQueue().dispatch();
    processSystems(root, world);

    // fixed step systems
    ctx.deltaTime = fixedStep;
    accumulator += deltaTime;
    const float accumulatorBackup = accumulator;

    FrameContext::global().setStage(FrameStage::Physics);
    m_services.eventQueue().dispatch();
    while (accumulator >= fixedStep)
    {
        processSystems(root, world);
        accumulator -= fixedStep;
    }

    FrameContext::global().setStage(FrameStage::Gameplay);
    m_services.eventQueue().dispatch();
    accumulator = accumulatorBackup;
    while (accumulator >= fixedStep)
    {
        processSystems(root, world);
        accumulator -= fixedStep;
    }

    FrameContext::global().setStage(FrameStage::Animation);
    m_services.eventQueue().dispatch();
    accumulator = accumulatorBackup;
    while (accumulator >= fixedStep)
    {
        processSystems(root, world);
        accumulator -= fixedStep;
    }

    ctx.deltaTime = deltaTime;
    FrameContext::global().setStage(FrameStage::Render);
    auto &stageSystems = m_services.compSys().getSystemRegistry().getStageSystems(FrameContext::currentStage());
    for (auto &sys : stageSystems)
    {
        sys->beginFrame();
    }
    m_services.eventQueue().dispatch();
    processSystems(root, world);
    for (auto &sys : stageSystems)
    {
        sys->endFrame();
    }

    FrameContext::global().setStage(FrameStage::End);
    m_services.eventQueue().dispatch();
    processSystems(root, world);

    m_services.compSys().removeScheduled();
}

Application &Engine::application()
{
    return *m_app;
}

void Engine::processSystems(Node &obj, World &world)
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
    auto *engine = static_cast<Engine*>(userData);
    engine->step();
}
