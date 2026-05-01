#pragma once

#include <cstddef>
#include <span>
#include <string>

class AssetId
{
    bool m_isNull = true;
    std::byte m_data[16] = {};
public:
    AssetId() = default;
    
    static AssetId generate();
    
    AssetId(std::span<const std::byte, 16> data)
    {
        std::copy(data.begin(), data.end(), std::begin(m_data));
        m_isNull = false;
    }

    bool operator==(const AssetId &other) const
    {
        return std::memcmp(m_data, other.m_data, 16) == 0;
    }
    
    bool isNull() const;
};
