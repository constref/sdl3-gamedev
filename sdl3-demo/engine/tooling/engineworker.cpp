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
	// create socket for engine->tooling data
	const std::string engineUrl = "NUBEEngine";

	shouldRun = engine->initialize(logW, logH, width, height);
	if (shouldRun)
	{
		// start up the pull socket for tooling events
		pullThread = std::thread([this, engineUrl]() {
			while (shouldRun)
			{
				HANDLE hPipe = CreateNamedPipeA(std::format("\\\\.\\pipe\\{}", engineUrl).c_str(), PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT,
					1, 1024 * 64, 1024 * 64, 0, NULL);
				BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
				if (connected)
				{
					const size_t bufferSize = 1024;
					uint8_t buffer[bufferSize];
					while (shouldRun)
					{
						uint32_t msgSize = 0;
						DWORD bytesRead = 0;
						BOOL success = ReadFile(hPipe, &msgSize, sizeof(uint32_t), &bytesRead, NULL);
						if (!success || bytesRead == 0)
						{
							break;
						}

						assert(msgSize <= bufferSize && "Message size too large for input buffer.");
						success = ReadFile(hPipe, buffer, msgSize, &bytesRead, NULL);
						if (success && bytesRead == msgSize)
						{
							auto envelope = NUBE::Interop::GetEngineEnvelope(buffer);
							switch (envelope->payload_type())
							{
								case NUBE::Interop::EngineMessage_EngineShutdownCommand:
								{
									pushEvent(ExitEvent());
									break;
								}
								case NUBE::Interop::EngineMessage_KeyboardEvent:
								{
									const NUBE::Interop::KeyboardEvent *keyEvent = envelope->payload_as_KeyboardEvent();
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
							}
						}
					}
				}
			}
			return 0;
		});

		// Push data into editor for INIT
		HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\NUBEEditor"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			using namespace NUBE::Interop;
			std::vector<uint64_t> texHandles = engine->getRenderer()->getSharedTextureHandles(editorPID);

			auto *builder = new flatbuffers::FlatBufferBuilder(1024);
			auto initDetails = NUBE::Interop::CreateInitializationDetails(*builder,
				texHandles.size(),
				builder->CreateVector(texHandles),
				builder->CreateString(engineUrl)
			);

			auto msg = NUBE::Interop::CreateEditorEnvelope(*builder, EditorMessage::EditorMessage_InitializationDetails, initDetails.Union());
			builder->FinishSizePrefixed(msg);
			auto span = builder->GetBufferSpan();

			DWORD bytesWritten = 0;
			BOOL success = WriteFile(hPipe, span.data(), span.size(), &bytesWritten, NULL);
			delete builder;
		}
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
