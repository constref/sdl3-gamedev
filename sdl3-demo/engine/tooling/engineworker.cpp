#include "engineworker.h"
#include <format>
#include <zmq.hpp>
#include <tooling/usd/usdprocessor.h>
#include <nube.pb.h>

#include "usd.pb.h"

EngineWorker::EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string &url)
{
    m_engine = std::make_unique<Engine>(std::move(app));
    m_usdProcessor = std::make_unique<usd::UsdProcessor>(nullptr);
    m_listening = false;
    m_running = false;
    this->editorPID = editorPID;
    this->url = url;
}

EngineWorker::~EngineWorker()
{
    m_listening = false;
    m_running = false;
}

void EngineWorker::start()
{
    m_listening = true;
    zmq::context_t ctx;

    // subscribe to editor event publisher
    m_subThread = std::thread([this, &ctx]()
    {
        zmq::socket_t sub(ctx, zmq::socket_type::sub);
        sub.connect(std::format("{}-EditorPub", url));
        sub.set(zmq::sockopt::subscribe, "");
        sub.set(zmq::sockopt::rcvtimeo, 1000);

        while (m_listening)
        {
            zmq::message_t msg;
            auto result = sub.recv(msg, zmq::recv_flags::none);
            if (result.has_value() && result.value() > 0)
            {
                NUBE::EditorEnvelope editorEnvelope;
                bool parseSuccess = editorEnvelope.ParseFromArray(msg.data(), msg.size());
                if (parseSuccess)
                {
                    switch (editorEnvelope.payload_case())
                    {
                        case NUBE::EditorEnvelope::kKeyboardEvent:
                        {
                            const auto &keyEvent = editorEnvelope.keyboardevent();
                            if (keyEvent.isdown())
                            {
                                pushEvent(KeyDown{.scancode = keyEvent.scancode()});
                            }
                            else
                            {
                                pushEvent(KeyUp{.scancode = keyEvent.scancode()});
                            }
                            break;
                        }
                        case NUBE::EditorEnvelope::kMouseMoveEvent:
                        {
                            const auto &mouseMoveEvent = editorEnvelope.mousemoveevent();
                            pushEvent(MouseMoveEvent{
                                .x = mouseMoveEvent.x(), .y = mouseMoveEvent.y(),
                                .xRel = mouseMoveEvent.xrel(), .yRel = mouseMoveEvent.yrel()
                            });
                            break;
                        }
                        case NUBE::EditorEnvelope::PAYLOAD_NOT_SET:
                            break;
                    }
                }
            }
        }
    });
    if (m_subThread.joinable())
    {
        m_subThread.detach();
    }

    // create socket for engine->tooling data
    zmq::socket_t rep = zmq::socket_t(ctx, zmq::socket_type::rep);
    rep.bind(url);

    const auto ack = [&rep]
    {
        zmq::message_t response("ACK");
        rep.send(response, zmq::send_flags::none);
    };

    // incoming editor requests
    while (m_listening)
    {
        zmq::message_t request;
        auto result = rep.recv(request, zmq::recv_flags::none);
        if (result.has_value())
        {
            NUBE::EditorEnvelope editorEnvelope;
            bool parseSuccess = editorEnvelope.ParseFromArray(request.data(), request.size());
            if (parseSuccess)
            {
                switch (editorEnvelope.payload_case())
                {
                    case NUBE::EditorEnvelope::kStartup:
                    {
                        uint32_t width = editorEnvelope.startup().width();
                        uint32_t height = editorEnvelope.startup().height();
                        m_engine->initialize(width, height, width, height);
                        // shared handles for GPU interop
                        std::vector<uint64_t> texHandles = m_engine->getRenderer()->getSharedTextureHandles(editorPID);

                        // send back the render-init response
                        auto *initDetails = new NUBE::InitializationDetails();
                        initDetails->set_engineurl("ipc://");
                        initDetails->set_maxframesinflight(texHandles.size());
                        for (uint64_t texHandle : texHandles)
                        {
                            initDetails->add_targethandles(texHandle);
                        }

                        NUBE::EngineEnvelope envelope;
                        envelope.set_allocated_initdetails(initDetails);
                        size_t size = envelope.ByteSizeLong();
                        std::vector<uint8_t> buffer(size);
                        bool success = envelope.SerializeToArray(buffer.data(), buffer.size());
                        if (success)
                        {
                            std::span<uint8_t> span(buffer.data(), buffer.size());
                            zmq::message_t response(span);
                            rep.send(response, zmq::send_flags::none);
                        }

                        m_running = true;
                        m_engineThread = std::thread([this]()
                        {
                            while (m_running)
                            {
                                processEvents();
                                m_engine->step();
                            }
                            m_engine->cleanup();
                        });
                        break;
                    }
                    case NUBE::EditorEnvelope::kShutdown:
                    {
                        m_running = false;
                        if (m_engineThread.joinable())
                        {
                            m_engineThread.join();
                        }
                        m_listening = false;
                        ack();
                        break;
                    }
                    case NUBE::EditorEnvelope::kBakeStage:
                    {
                        m_usdProcessor->openStage(editorEnvelope.bakestage().usdpath());
                        pushEvent(usd::BakeStageEvent(m_usdProcessor.get()));
                        ack();
                        break;
                    }
                    default: Logger::error(this, "Unrecognized command received by editor");
                }
            }
        }
    }
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
            //const ResizeEvent &event = std::get<ResizeEvent>(e);
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
            m_listening = false;
            m_running = false;
        }
        else if (std::holds_alternative<usd::BakeStageEvent>(e))
        {
            NodeHandle hRoot = m_engine->application().getRoot();
            Node &root = serv.world().getNode(hRoot);
            m_usdProcessor->bakeStage(root, serv);
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
