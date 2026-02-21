#include "engineworker.h"

#include <format>
#include <engine_generated.h>

EngineWorker::EngineWorker(std::unique_ptr<Engine> engine, int editorPID, const std::string &editorUrl, int logW, int logH, int width, int height)
{
	this->engine = std::move(engine);
	this->logW = logW;
	this->logH = logH;
	this->width = width;
	this->height = height;
	this->shouldRun = false;
	this->editorPID = editorPID;
	this->editorUrl = editorUrl;
}

EngineWorker::~EngineWorker()
{
	shouldRun = false;
}

void EngineWorker::start()
{
	zmq::context_t ctx;

	// create socket for engine->tooling data
	//push = zmq::socket_t(ctx, zmq::socket_type::push);
	//push.connect("tcp://127.0.0.1:0");

	pull = zmq::socket_t(ctx, zmq::socket_type::pull);
	pull.bind("tcp://127.0.0.1:0");
	const std::string engineUrl = pull.get(zmq::sockopt::last_endpoint);

	shouldRun = engine->initialize(logW, logH, width, height);
	if (shouldRun)
	{
		// start up the pull socket for tooling events
		pullThread = std::thread([this]() {
			while (shouldRun)
			{
				zmq::message_t msg;
				pull.recv(msg);

				auto envelope = NUBE::Interop::GetEngineEnvelope(msg.data());
				const NUBE::Interop::KeyboardEvent *keyEvent = envelope->payload_as_KeyboardEvent();

				if (keyEvent->is_down())
				{
					pushEvent(KeyDown{ .scancode = keyEvent->scancode() });
				}
				else
				{

					pushEvent(KeyUp{ .scancode = keyEvent->scancode() });
				}
			}
		});
		using namespace NUBE::Interop;
		std::vector<uint64_t> texHandles = engine->getRenderer()->getSharedTextureHandles(editorPID);

		auto *builder = new flatbuffers::FlatBufferBuilder(1024);
		auto initDetails = NUBE::Interop::CreateInitializationDetails(*builder,
			engine->getRenderer()->getMaxFramesInFlight(),
			builder->CreateVector(texHandles),
			builder->CreateString(engineUrl)
		);

		auto msg = NUBE::Interop::CreateEditorEnvelope(*builder, EditorMessage::EditorMessage_InitializationDetails, initDetails.Union());
		builder->Finish(msg);
		auto span = builder->GetBufferSpan();

		// send the initial handshake data to the editor
		zmq::socket_t request(ctx, zmq::socket_type::req);
		request.connect(editorUrl);
		request.send(span.data(), span.size());
	}
	while (shouldRun)
	{
		processEvents();
		engine->step();
	}
}

void EngineWorker::processEvents()
{
	PlatformEvent e;
	while (eventBuffer.get(e))
	{
		// handle external events
		if (std::holds_alternative<InteropMessage>(e))
		{
		}
		else if (std::holds_alternative<ApplicationEnteredBackground>(e))
		{
			//app.pause();
		}
		else if (std::holds_alternative<ApplicationEnteredForeground>(e))
		{
			//app.resume();
		}
		else if (std::holds_alternative<KeyUp>(e))
		{
			const KeyUp &keyEvent = std::get<KeyUp>(e);
			Services &serv = engine->getServices();
			serv.eventQueue().enqueue<KeyUpEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
		}
		else if (std::holds_alternative<KeyDown>(e))
		{
			const KeyDown &keyEvent = std::get<KeyDown>(e);
			Services &serv = engine->getServices();
			serv.eventQueue().enqueue<KeyDownEvent>(serv.inputState().getFocusTarget(), 0, keyEvent.scancode);
		}
		else if (std::holds_alternative<MouseMoveEvent>(e))
		{
			//const MouseMoveEvent &mouseMoveEvent = std::get<MouseMoveEvent>(e);
			//app.onMouseMove(mouseMoveEvent.x, mouseMoveEvent.y);
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
		else if (std::holds_alternative<ExitEvent>(e))
		{
			shouldRun = false;
		}
	}
}

void EngineWorker::pushEvent(const PlatformEvent &event)
{
	eventBuffer.add(event);
}

Engine &EngineWorker::getEngine()
{
	return *engine;
}

void EngineWorker::processInteropMessage(uint32_t msgType)
{
	pushEvent(InteropMessage{ .messageType = msgType });
}
