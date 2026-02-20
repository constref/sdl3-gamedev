#pragma once

#include <stdint.h>

struct RenderInfo
{
	uint32_t framesInFlight;
	uint64_t renderTargetSize;
	uint32_t workCompletePoolSize;
	uint32_t imageReadyPoolSize;
};

struct ExportedResources
{
	uint64_t textureMemoryHandle;
};

