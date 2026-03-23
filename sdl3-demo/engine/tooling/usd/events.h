#pragma once

#include <string>
#include <messaging/event.h>

namespace usd
{
class UsdProcessor;

struct BakeStageEvent
{
	usd::UsdProcessor *currentProcessor;
};
}