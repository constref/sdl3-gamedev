#pragma once
#include <cstdint>

namespace usd
{

#pragma pack(push, 8)
struct Prim
{
    uint32_t id;
    uint32_t parentId;
    const char *name;
    const char *type;
    const char *path;
};
#pragma pack(pop)

}
