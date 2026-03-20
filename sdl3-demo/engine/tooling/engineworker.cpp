#include "engineworker.h"
#include <format>

#include <zmq.hpp>

#include "usd/usdprocessor.h"

//static zmq::context_t g_context;

EngineWorker::EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string& url)
{
    m_engine = std::make_unique<Engine>(std::move(app));
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
    const std::string engineUrl = "NUBEEngine";

    // pull socket for high-frequency, low-latency data
    m_pullThread = std::thread([this, engineUrl]()
    {
        HANDLE hPipe = CreateNamedPipeA(std::format("\\\\.\\pipe\\{}", engineUrl).c_str(),
                                        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                        1, 1024 * 64, 1024 * 64, 0, NULL);

        OVERLAPPED ov{};
        ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        // perform async connect
        BOOL connected = ConnectNamedPipe(hPipe, &ov);
        if (!connected)
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                WaitForSingleObject(ov.hEvent, INFINITE);
            }
            else
            {
                CloseHandle(ov.hEvent);
                CloseHandle(hPipe);
                return;
            }
        }
        ResetEvent(ov.hEvent);

        Logger::info(this, "Engine runtime server started, listening for incoming commands...");
        while (m_listening)
        {
            constexpr size_t headerSize{sizeof(uint32_t)};
            constexpr size_t bufferSize{1024uz * 64uz};
            uint8_t buffer[bufferSize];
            uint32_t msgSize = 0;
            DWORD bytesRead = 0;

            // read the message size, followed by the payload
            BOOL success = ReadFile(hPipe, &msgSize, headerSize, &bytesRead, &ov);
            if (!success)
            {
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    WaitForSingleObject(ov.hEvent, INFINITE);
                    if (!GetOverlappedResult(hPipe, &ov, &bytesRead, false))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            Logger::info(this, std::format("{} header-bytes read", bytesRead));
            if (bytesRead != headerSize)
            {
                Logger::error(this, "Invalid header size read");
                break;
            }

            ResetEvent(ov.hEvent);
            // read payload
            success = ReadFile(hPipe, &buffer, msgSize, &bytesRead, &ov);
            if (!success)
            {
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    WaitForSingleObject(ov.hEvent, INFINITE);
                    if (!GetOverlappedResult(hPipe, &ov, &bytesRead, false))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            // message read incomplete
            Logger::info(this, std::format("{} message-bytes read", bytesRead));
            if (bytesRead != msgSize)
            {
                Logger::error(this, "Invalid message size read");
                break;
            }

            // handle specific message
            using namespace NUBE::Interop;
            auto envelope = Piped::GetPipedEnvelope(buffer);
            switch (envelope->payload_type())
            {
                case Piped::PipedMessage_KeyboardEvent:
                {
                    const Piped::KeyboardEvent* keyEvent = envelope->payload_as_KeyboardEvent();
                    if (keyEvent->is_down())
                    {
                        pushEvent(KeyDown{.scancode = keyEvent->scancode()});
                    }
                    else
                    {
                        pushEvent(KeyUp{.scancode = keyEvent->scancode()});
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
                default: {}
            }
        }
        CancelIoEx(hPipe, nullptr);
        CloseHandle(hPipe);
        CloseHandle(ov.hEvent);
        Logger::info(this, "Engine runtime server finished.");
    });
    if (m_pullThread.joinable())
    {
        m_pullThread.detach();
    }

    // create socket for engine->tooling data
    zmq::context_t ctx;
    zmq::socket_t rep = zmq::socket_t(ctx, ZMQ_REP);
    rep.bind(url);

    usd::UsdProcessor usdSystem;
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
                                                                   builder.CreateString(engineUrl));

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
                    zmq::message_t response(0);
                    rep.send(response, zmq::send_flags::none);
                    
                    m_running = false;
                    if (m_engineThread.joinable())
                    {
                        m_engineThread.detach();
                    }
                    break;
                }
                case EditorMessage_USD_CreateStageCommand:
                {
                    const auto* e = envelope->payload_as_USD_CreateStageCommand();
                    usdSystem.createStage(e->path()->c_str());
                    zmq::message_t response("ACK");
                    rep.send(response, zmq::send_flags::none);
                    break;
                }
                case EditorMessage_USD_SaveStageCommand:
                {
                    const auto* e = envelope->payload_as_USD_SaveStageCommand();
                    usdSystem.saveStage();
                    zmq::message_t response("ACK");
                    rep.send(response, zmq::send_flags::none);
                    break;
                }
                case EditorMessage_USD_AddLayerCommand:
                {
                    const auto* e = envelope->payload_as_USD_AddLayerCommand();
                    usdSystem.addLayer(e->path()->c_str());
                    zmq::message_t response("ACK");
                    rep.send(response, zmq::send_flags::none);
                    break;
                }
                case EditorMessage_USD_AddPrimCommand:
                {
                    const auto* e = envelope->payload_as_USD_AddPrimCommand();
                    usdSystem.addPrim(e->path()->c_str(), e->type()->c_str());
                    zmq::message_t response("ACK");
                    rep.send(response, zmq::send_flags::none);
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
    Services& serv = m_engine->getServices();
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
            //	app.onMouseButtonDown(mouseButtonEvent.buttonIndex);
            //}
            //else
            //{
            //	app.onMouseButtonUp(mouseButtonEvent.buttonIndex);
            //}
        }
        else if (std::holds_alternative<ResizeEvent>(e))
        {
            //const ResizeEvent &event = std::get<ResizeEvent>(e);
            //app.onResizeRenderer(event.x, event.y, event.width, event.height);
        }
        else if (std::holds_alternative<ApplicationEnteredBackground>(e))
        {
            //app.pause();
        }
        else if (std::holds_alternative<ApplicationEnteredForeground>(e))
        {
            //app.resume();
        }
        else if (std::holds_alternative<ExitEvent>(e))
        {
            Logger::info(this, "Engine exit event received, stopping run-loop");
            m_listening = false;
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

void EngineWorker::pushEvent(const PlatformEvent& event)
{
    eventBuffer.add(event);
}

Engine& EngineWorker::getEngine()
{
    return *m_engine;
}
