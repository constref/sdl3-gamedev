#include <engine_generated.h>
#include "engineworker.h"
#include <format>
#include <zmq.hpp>
#include <tooling/usd/usdprocessor.h>

EngineWorker::EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string& url)
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

    m_pullThread = std::thread([this, &ctx]()
    {
        zmq::socket_t sub(ctx, zmq::socket_type::sub);
        sub.connect(std::format("{}-EditorPub", url));
        sub.set(zmq::sockopt::subscribe, "");
		sub.set(zmq::sockopt::rcvtimeo, 100);

        while (m_listening)
        {
            zmq::message_t msg;
            auto result = sub.recv(msg, zmq::recv_flags::none);
            if (result.has_value() && result.value() > 0)
            {
                using namespace NUBE::Interop;
                auto envelope = Piped::GetPipedEnvelope(msg.data());
                switch (envelope->payload_type())
                {
                    case Piped::PipedMessage_KeyboardEvent:
                    {
						Logger::info(this, "Received keyboard event from editor");
                        const Piped::KeyboardEvent* keyEvent = envelope->payload_as_KeyboardEvent();
                        if (keyEvent->is_down())
                        {
                            pushEvent(KeyDown{ .scancode = keyEvent->scancode() });
                        }
                        else
                        {
                            pushEvent(KeyUp{ .scancode = keyEvent->scancode() });
                        }
                        break;
                    }
                    case Piped::PipedMessage::PipedMessage_MouseMoveEvent:
                    {
                        const auto* mouseMoveEvent = envelope->payload_as_MouseMoveEvent();
                        pushEvent(MouseMoveEvent{
                            .x = mouseMoveEvent->x(), .y = mouseMoveEvent->y(), .xRel = mouseMoveEvent->x_rel(),
                            .yRel = mouseMoveEvent->y_rel()
                        });
                        break;
                    }
                    default:
                    {
                    }
                }
            }
        }
    });
    if (m_pullThread.joinable())
    {
        m_pullThread.detach();
    }

    // create socket for engine->tooling data
    zmq::socket_t rep = zmq::socket_t(ctx, zmq::socket_type::rep);
    rep.bind(url);

    const auto ack = [&rep]
    {
        zmq::message_t response("ACK");
        rep.send(response, zmq::send_flags::none);
    };

    while (m_listening)
    {
        zmq::message_t request;
        auto msg = rep.recv(request, zmq::recv_flags::none);

        using namespace NUBE::Interop;
        if (msg.has_value())
        {
            auto envelope = GetEditorEnvelope(request.data());
            switch (envelope->payload_type())
            {
                case EditorMessage_EngineStartupCommand:
                {
                    m_engine->initialize(1920, 1080, 1920, 1080);
                    // shared handles for GPU interop
                    std::vector<uint64_t> texHandles = m_engine->getRenderer()->getSharedTextureHandles(editorPID);

                    // send back the render-init response
                    flatbuffers::FlatBufferBuilder builder(1024);
                    auto initDetails = CreateInitializationDetails(builder, texHandles.size(),
                                                                   builder.CreateVector(texHandles),
                                                                   builder.CreateString("ipc://"));

                    auto env = CreateEngineEnvelope(builder, EngineMessage_InitializationDetails, initDetails.Union());
                    builder.Finish(env);
                    auto span = builder.GetBufferSpan();
                    zmq::message_t response(span);
                    rep.send(response, zmq::send_flags::none);

                    m_running = true;
                    m_engineThread = std::thread([this]()
                    {
                        while (m_running)
                        {
                            processEvents();
                            m_engine->step();
                        }
                        m_engine->cleanup();
                        Logger::info(this, "Engine worker shutting down");
                    });
                    break;
                }
                case EditorMessage_EngineShutdownCommand:
                {
                    m_running = false;
                    if (m_engineThread.joinable())
                    {
                        m_engineThread.detach();
                    }
                    ack();
                    break;
                }
                case EditorMessage_USD_CreateStageCommand:
                {
                    const auto* e = envelope->payload_as_USD_CreateStageCommand();
                    m_usdProcessor->createStage(e->path()->str());
                    ack();
                    break;
                }
                case EditorMessage_USD_OpenStageCommand:
                {
                    const auto* e = envelope->payload_as_USD_OpenStageCommand();
                    m_usdProcessor->openStage(e->path()->str());
                    ack();
                    break;
                }
                case EditorMessage_USD_SaveStageCommand:
                {
                    m_usdProcessor->saveStage();
                    ack();
                    break;
                }
                case EditorMessage_USD_AddLayerCommand:
                {
                    const auto* e = envelope->payload_as_USD_AddLayerCommand();
                    ack();
                    break;
                }
                case EditorMessage_USD_AddPrimCommand:
                {
                    const auto* e = envelope->payload_as_USD_AddPrimCommand();
                    ack();
                    break;
                }
                case EditorMessage_USD_BakeStageCommand:
                {
                    pushEvent(usd::BakeStageEvent(m_usdProcessor.get()));
                    ack();
                    break;
                }
                default:
                {
                    Logger::error(this, "Unrecognized command received by editor");
                };
            }
        }
    }


    // Push data into editor for INIT
    /*
        HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\NUBEEditor"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE)
        {
            using namespace NUBE::Interop;
            std::vector<uint64_t> texHandles = engine->getRenderer()->getSharedTextureHandles(editorPID);

            auto* builder = new flatbuffers::FlatBufferBuilder(1024);
            auto initDetails = NUBE::Interop::CreateInitializationDetails(*builder,
                                                                          texHandles.size(),
                                                                          builder->CreateVector(texHandles),
                                                                          builder->CreateString(engineUrl)
            );

            auto msg = NUBE::Interop::CreateEditorEnvelope(*builder, EditorMessage::EditorMessage_InitializationDetails,
                                                           initDetails.Union());
            builder->FinishSizePrefixed(msg);
            auto span = builder->GetBufferSpan();

            DWORD bytesWritten = 0;
            BOOL success = WriteFile(hPipe, span.data(), span.size(), &bytesWritten, NULL);
            delete builder;
        }
    }
    */
}

void EngineWorker::processEvents()
{
    PlatformEvent e;
    Services& serv = m_engine->services();
    while (eventBuffer.get(e))
    {
        // handle external events
        if (std::holds_alternative<KeyDown>(e))
        {
            const KeyDown& keyEvent = std::get<KeyDown>(e);
            serv.eventQueue().enqueue<KeyDownEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
        }
        else if (std::holds_alternative<KeyUp>(e))
        {
            const KeyUp& keyEvent = std::get<KeyUp>(e);
            serv.eventQueue().enqueue<KeyUpEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
        }
        else if (std::holds_alternative<MouseMoveEvent>(e))
        {
            const MouseMoveEvent& mouseEvent = std::get<MouseMoveEvent>(e);
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
            Node& root = serv.world().getNode(hRoot);
            m_usdProcessor->bakeStage(root, serv);
            //serv.eventQueue().enqueue<usd::BakeStageEvent>(NodeHandle{}, 0, m_usdProcessor.get());
        }
    }
}

void EngineWorker::pushEvent(const PlatformEvent& event)
{
    eventBuffer.add(event);
}

Engine& EngineWorker::getEngine()
{
    return *m_engine;
}
