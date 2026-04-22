#pragma once
#include <cstdint>

namespace usd
{
typedef void(__stdcall *ObjectsChangedFunc)(const uint8_t *data, size_t size);
}
