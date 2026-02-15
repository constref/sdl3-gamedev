#pragma once

#include <stdint.h>

struct RenderInfo
{
	uint32_t framesInFlight;
};

struct ExportedResources
{
	intptr_t textureMemoryHandle;
	intptr_t waitSemaphoreHandle;
	intptr_t signalSemaphoreHandle;
};

