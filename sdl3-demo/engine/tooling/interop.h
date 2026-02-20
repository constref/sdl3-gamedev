#pragma once

#include <stdint.h>

struct MsgBuffer
{
	uint8_t *data;
	size_t byteSize;
};

class Interop
{
public:
	Interop();
	virtual ~Interop();
	MsgBuffer processMessage(int msgId);
};
