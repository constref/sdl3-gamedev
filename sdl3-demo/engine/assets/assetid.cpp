#include "assetid.h"

#include <random>
#include <uuid.h>

AssetId AssetId::generate()
{
    static std::mt19937 rng;
    uuids::uuid_random_generator gen(rng);
    return AssetId(gen().as_bytes());
}

bool AssetId::isNull() const
{
    return m_isNull;
}
