#include "interop.h"

//#include <engine_generated.h>

Interop::Interop()
{
}

Interop::~Interop()
{
}

enum class InteropMessage
{
	GetRenderInfo = 1
};

MsgBuffer Interop::processMessage(int msgType)
{
	InteropMessage msg = static_cast<InteropMessage>(msgType);

	switch (msg)
	{
		case InteropMessage::GetRenderInfo:
		{
		}
	}
	return MsgBuffer{ .data = 0, .byteSize = 0 };
}
