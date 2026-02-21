#include "engineworker.h"

#include <zmq.hpp>
#include <engine_generated.h>

EngineWorker::EngineWorker(std::unique_ptr<Engine> engine, int editorPID, int logW, int logH, int width, int height) : engine(std::move(engine))
{
	this->logW = logW;
	this->logH = logH;
	this->width = width;
	this->height = height;
	this->shouldRun = false;
	this->editorPID = editorPID;
}

EngineWorker::~EngineWorker()
{
	shouldRun = false;
}

void EngineWorker::start()
{
	zmq::context_t ctx;
	zmq::socket_t request(ctx, zmq::socket_type::req);
	request.connect("tcp://localhost:5555");

	shouldRun = engine->initialize(logW, logH, width, height);
	if (shouldRun)
	{
		std::vector<uint64_t> texHandles = engine->getRenderer()->getSharedTextureHandles(editorPID);
		auto *builder = new flatbuffers::FlatBufferBuilder(1024);
		flatbuffers::Offset<NUBE::Interop::RenderInfo> rendInfo =
			NUBE::Interop::CreateRenderInfo(*builder,
				engine->getRenderer()->getMaxFramesInFlight(),
				builder->CreateVector(texHandles));

		builder->Finish(rendInfo);
		auto span = builder->GetBufferSpan();
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
