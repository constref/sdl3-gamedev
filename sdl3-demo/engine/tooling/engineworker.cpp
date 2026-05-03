#include "engineworker.h"
#include "nodehandle.h"
#include <format>
#include <memory>
#include <persistence/project.h>
#include <uuid.h>
#include <zmq.hpp>

#include <nube.pb.h>
#include <usd.pb.h>

#include <components/meshcomponent.h>

EngineWorker::EngineWorker(std::unique_ptr<Application> app, int editorPID, const std::string &url)
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
    zmq::context_t ctx;

    // socket for low-latency events (input)
    zmq::socket_t sub(ctx, zmq::socket_type::sub);
    sub.connect(std::format("{}-EditorPub", url));
    sub.set(zmq::sockopt::subscribe, "");
    sub.set(zmq::sockopt::rcvtimeo, 1000);

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
        // input events
        zmq::message_t inputMsg;
        auto msgReceived = sub.recv(inputMsg, zmq::recv_flags::dontwait);
        if (msgReceived)
        {
            NUBE::EditorEnvelope editorEnvelope;
            bool parseSuccess = editorEnvelope.ParseFromArray(inputMsg.data(), inputMsg.size());
            if (parseSuccess)
            {
                Services &services = m_engine->services();
                switch (editorEnvelope.payload_case())
                {
                    case NUBE::EditorEnvelope::kKeyboardEvent:
                    {
                        const auto &keyEvent = editorEnvelope.keyboardevent();
                        if (keyEvent.isdown())
                        {
                            services.eventQueue().enqueue<KeyDownEvent>(services.inputState().getFocusTarget(), 0,
                                                                        keyEvent.scancode());
                        }
                        else
                        {
                            services.eventQueue().enqueue<KeyUpEvent>(services.inputState().getFocusTarget(), 0,
                                                                      keyEvent.scancode());
                        }
                        break;
                    }
                    case NUBE::EditorEnvelope::kMouseMoveEvent:
                    {
                        const auto &mouseEvent = editorEnvelope.mousemoveevent();
                        services.eventQueue().enqueue<MouseMotionEvent>(services.inputState().getFocusTarget(), 0,
                                                                        mouseEvent.x(), mouseEvent.y(),
                                                                        mouseEvent.xrel(), mouseEvent.yrel());
                        break;
                    }
                }
            }
        }

        // incoming editor requests
        zmq::message_t request;
        auto result = rep.recv(request, zmq::recv_flags::dontwait);
        if (result)
        {
            NUBE::EditorEnvelope editorEnvelope;
            bool responded = false;
            bool parseSuccess = editorEnvelope.ParseFromArray(request.data(), request.size());
            if (parseSuccess)
            {
                Services &services = m_engine->services();
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
                        responded = true;
                        break;
                    }
                    case NUBE::EditorEnvelope::kLoadMesh:
                    {
                        const auto &cmd = editorEnvelope.loadmesh();
                        auto assetId = uuids::uuid::from_string(cmd.assetid());
                        assert(assetId.has_value() && "Invalid asset-id UUID provided");
                        auto mesh = persistence::readMeeshFile(cmd.assetid());
                        services.assetManager().loadMesh(assetId.value(), std::move(mesh));
                        break;
                    }
                    case NUBE::EditorEnvelope::kCreateNode:
                    {
                        NodeHandle hNode = services.world().createNode();
                        NUBE::NodeHandle *handle = new NUBE::NodeHandle;
                        handle->set_index(hNode.index());
                        handle->set_generation(hNode.generation());

                        NUBE::EngineEnvelope envelope;
                        envelope.set_allocated_nodehandle(handle);

                        size_t size = envelope.ByteSizeLong();
                        std::vector<uint8_t> buffer(size);
                        bool success = envelope.SerializeToArray(buffer.data(), buffer.size());
                        if (success)
                        {
                            std::span<uint8_t> span(buffer.data(), buffer.size());
                            zmq::message_t response(span);
                            rep.send(response, zmq::send_flags::none);
                            responded = true;
                        }
                        break;
                    }
                    case NUBE::EditorEnvelope::kAttachMesh:
                    {
                        auto targetHandle = editorEnvelope.attachmesh().targetnode();
                        NodeHandle hNode(targetHandle.index(), targetHandle.generation());
                        Node &node = services.world().getNode(hNode);

                        auto meshId = uuids::uuid::from_string(editorEnvelope.attachmesh().assetid());
                        if (!meshId.has_value())
                        {
                            Logger::error(this, "The provided meshId is invalid");
                        }
                        else
                        {
                            auto &meshComp = services.compSys().addComponent<MeshComponent>(node, meshId.value());
                        }
                        break;
                    }
                    case NUBE::EditorEnvelope::kShutdown:
                    {
                        Logger::info(this, "Engine exit event received, stopping run-loop");
                        m_running = false;
                        m_listening = false;
                    }
                    default:
                    {
                        Logger::warn(this, "Unhandled worker event");
                    }
                }
            }
            if (!responded)
            {
                ack();
            }
        }

        // run the engine
        if (m_running)
        {
            m_engine->step();
        }
    }
    m_engine->cleanup();
}

Engine &EngineWorker::getEngine() { return *m_engine; }
