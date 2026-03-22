#include "engineworker.h"
#include <format>

EngineWorker::EngineWorker(std::unique_ptr<Application> app)
{
    m_engine = std::make_unique<Engine>(std::move(app));
    m_running = false;
}

EngineWorker::~EngineWorker()
{
    m_running = false;
    if (m_engineThread.joinable())
    {
        m_engineThread.join();
    }
}

void EngineWorker::start(HWND hWnd, int width, int height)
{
    m_engine->initialize(width, height, width, height, hWnd);
    
    m_running = true;
    m_engineThread = std::thread([this]
    {
        while (m_running)
        {
            processEvents();
            m_engine->step();
        }
    });
}

void EngineWorker::processEvents()
{
    PlatformEvent e;
    Services &serv = m_engine->services();
    while (eventBuffer.get(e))
    {
        // handle external events
        if (std::holds_alternative<KeyDown>(e))
        {
            const KeyDown &keyEvent = std::get<KeyDown>(e);
            serv.eventQueue().enqueue<KeyDownEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
        }
        else if (std::holds_alternative<KeyUp>(e))
        {
            const KeyUp &keyEvent = std::get<KeyUp>(e);
            serv.eventQueue().enqueue<KeyUpEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
        }
        else if (std::holds_alternative<MouseMoveEvent>(e))
        {
            const MouseMoveEvent &mouseEvent = std::get<MouseMoveEvent>(e);
            serv.eventQueue().enqueue<MouseMotionEvent>(serv.inputState().getFocusTarget(), 0, mouseEvent.x,
                                                        mouseEvent.y, mouseEvent.xRel, mouseEvent.yRel);
        }
        else if (std::holds_alternative<MouseButtonEvent>(e))
        {
            //const MouseButtonEvent &mouseButtonEvent = std::get<MouseButtonEvent>(e);
            //if (mouseButtonEvent.isDown)
            //{
            //	m_app.onMouseButtonDown(mouseButtonEvent.buttonIndex);
            //}
            //else
            //{
            //	m_app.onMouseButtonUp(mouseButtonEvent.buttonIndex);
            //}
        }
        else if (std::holds_alternative<ResizeEvent>(e))
        {
            const ResizeEvent &event = std::get<ResizeEvent>(e);
            //m_app.onResizeRenderer(event.x, event.y, event.width, event.height);
        }
        else if (std::holds_alternative<ApplicationEnteredBackground>(e))
        {
            //m_app.pause();
        }
        else if (std::holds_alternative<ApplicationEnteredForeground>(e))
        {
            //m_app.resume();
        }
        else if (std::holds_alternative<ExitEvent>(e))
        {
            Logger::info(this, "Engine exit event received, stopping run-loop");
            m_running = false;
        }
        else if (std::holds_alternative<usd::CreateStageEvent>(e))
        {
            serv.eventQueue().enqueue<
                usd::CreateStageEvent>(NodeHandle{}, 0, std::get<usd::CreateStageEvent>(e).path());
        }
        else if (std::holds_alternative<usd::SaveStageEvent>(e))
        {
            serv.eventQueue().enqueue<usd::SaveStageEvent>(NodeHandle{}, 0);
        }
        else if (std::holds_alternative<usd::AddLayerEvent>(e))
        {
            serv.eventQueue().enqueue<usd::AddLayerEvent>(NodeHandle{}, 0, std::get<usd::AddLayerEvent>(e).path());
        }
    }
}

void EngineWorker::pushEvent(const PlatformEvent &event)
{
    eventBuffer.add(event);
}

Engine& EngineWorker::getEngine()
{
    return *m_engine;
}
