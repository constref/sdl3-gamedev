#pragma once

#include <cstddef>
#include <cstdint>

class NodeHandle
{
    size_t m_index;
    uint32_t m_generation;

public:
    NodeHandle() : m_index(0), m_generation(0) {}
    NodeHandle(size_t idx, uint32_t gen) : m_index(idx), m_generation(gen) {}

    auto index() const { return m_index; }
    auto generation() const { return m_generation; }
    bool isValid() const { return m_generation != 0; }

    bool operator==(const NodeHandle &other) { return m_index == other.m_index && m_generation == other.m_generation; }
};
