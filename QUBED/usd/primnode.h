#pragma once

#include <QString>
#include <vector>
#include <pxr/usd/sdf/path.h>

class PrimNode
{
    QString m_name;
    pxr::SdfPath m_path;
    PrimNode *m_parent;
    std::vector<PrimNode *> m_children;

public:
    PrimNode(const std::string &name, PrimNode *parent);

    auto &children() { return m_children; }
    auto &name() const { return m_name; }
};