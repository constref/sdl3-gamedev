#include "engineworker.h"
#include "persistence/project.h"
#include "uuid.h"
#include <format>
#include <zmq.hpp>

#include <nube.pb.h>
#include <usd.pb.h>

static AtomicRingBuffer<NUBE::EditorEnvelope, 64> g_inputEvents;
static AtomicRingBuffer<NUBE::EditorEnvelope, 64> g_envelopeBuffer;

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
			g_inputEvents.add(editorEnvelope);
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
	    bool responded = false;
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
			responded = true;
			break;
		    }
		    default:
		    {
			g_envelopeBuffer.add(editorEnvelope);
		    }
		}
	    }
	    if (!responded)
	    {
		ack();
	    }
	}
    }
    if (m_engineThread.joinable())
    {
	m_engineThread.join();
    }
}

void EngineWorker::processEvents()
{
    Services &serv = m_engine->services();
    NUBE::EditorEnvelope ee;

    // drain input event buffer first
    while (g_inputEvents.get(ee))
    {
	using namespace NUBE;
	switch (ee.payload_case())
	{
	    case EditorEnvelope::kKeyboardEvent:
	    {
		const auto &keyEvent = ee.keyboardevent();
		if (keyEvent.isdown())
		{
		    serv.eventQueue().enqueue<KeyDownEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode());
		}
		else
		{
		    serv.eventQueue().enqueue<KeyUpEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode());
		}
		break;
	    }
	    case EditorEnvelope::kMouseMoveEvent:
	    {
		const auto &mouseEvent = ee.mousemoveevent();
		serv.eventQueue().enqueue<MouseMotionEvent>(serv.inputState().getFocusTarget(), 0, mouseEvent.x(),
							    mouseEvent.y(), mouseEvent.xrel(), mouseEvent.yrel());
		break;
	    }
	}
    }

    // process all other engine events
    while (g_envelopeBuffer.get(ee))
    {
	using namespace NUBE;
	switch (ee.payload_case())
	{
	    case EditorEnvelope::kLoadMesh:
	    {
		const auto &cmd = ee.loadmesh();
		auto assetId = uuids::uuid::from_string(cmd.assetid());
		assert(assetId.has_value() && "Invalid asset-id UUID provided");
		auto mesh = persistence::readMeeshFile(cmd.assetid());
		serv.assetManager().loadMesh(assetId.value(), std::move(mesh));
		break;
	    }
	    case EditorEnvelope::kShutdown:
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
}

Engine &EngineWorker::getEngine() { return *m_engine; }
