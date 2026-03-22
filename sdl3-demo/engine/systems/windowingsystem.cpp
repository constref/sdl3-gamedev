#include "windowingsystem.h"

#include <SDL3/SDL.h>
#include <messaging/events.h>
#include <inputstate.h>
#include "tooling/events.h"

WindowingSystem::WindowingSystem(Services& services) : System(services)
{
    m_width = 0;
    m_height = 0;
    m_window = nullptr;
    m_relativeMouseMode = true;
    
    if constexpr (Config::IsToolingMode())
    {
        services.eventQueue().dispatcher.registerHandler<MouseRelativeModeToggleEvent>(this);
    }
}

WindowingSystem::~WindowingSystem()
{
    shutdown();
}

void WindowingSystem::onEvent(NodeHandle target, const MouseRelativeModeToggleEvent& event)
{
    m_relativeMouseMode = !m_relativeMouseMode;
    SDL_SetWindowRelativeMouseMode(m_window, m_relativeMouseMode);
}

bool WindowingSystem::initialize(int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        return false;
    }

    m_window = SDL_CreateWindow("NUBE Engine", width, height, SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), nullptr);
        shutdown();
        return false;
    }
    m_width = width;
    m_height = height;
    SDL_SetWindowRelativeMouseMode(m_window, m_relativeMouseMode);
    return true;
}

void WindowingSystem::shutdown() const
{
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

SDL_Window* WindowingSystem::window() const
{
    return m_window;
}

void WindowingSystem::update(Node& node)
{
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
            {
                services.eventQueue().enqueue<ShutdownEvent>(NodeHandle{}, 0);
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                //sdlState.width = event.window.data1;
                //sdlState.height = event.window.data2;
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            {
                // ignore repeat key-down signals while holding (prevent event spam)
                if (!event.key.repeat)
                {
                    services.eventQueue().enqueue<KeyDownEvent>(services.inputState().getFocusTarget(), 0,
                                                                event.key.scancode);
                }
                break;
            }
            case SDL_EVENT_KEY_UP:
            {
                services.eventQueue().enqueue<KeyUpEvent>(services.inputState().getFocusTarget(), 0,
                                                          event.key.scancode);
                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                services.eventQueue().enqueue<MouseMotionEvent>(services.inputState().getFocusTarget(), 0, 
                    event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            }
        }
    }
}
