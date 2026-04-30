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
#include <systems/d3d12/d3d12rendersystem.h>
#include <prototypeinstancer.h>
#include <rendering/renderer.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

class Engine
{
    std::unique_ptr<Application> m_app;
    uint64_t prevTime;
    bool debugMode;
    bool running;
    constexpr static bool clampDeltaTime = true;

    const float fixedStep = 1.0f / 120.0f;
    const float dtThreshold = 1.0f / 30.0f;
    float accumulator;
    long frameCount;
    double globalTime;

    // core m_services
    AssetManager assetManager;
    ComponentSystems compSys;
    EventQueue eventQueue;
    World world;
    InputState inputState;
    PrototypeInstancer protoInstancer;
    Services m_services;
    SDLState sdlState;
    Renderer *renderer;

public:
    Engine(std::unique_ptr<Application> app);
    ~Engine();

    bool initialize(int logW, int logH, int width, int height);
    Renderer *getRenderer() const;
    Services& services();
    void cleanup();
    void run();
    void start();
    void stop();
    void clear();
    void step();
    
    Application &application();

private:
    static void emIterate(void *userData);
    void processSystems(Node &obj, World &world);
};
