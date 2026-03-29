#pragma once

#include <QString>
#include <vector>
#include <tooling/usd.h>
#include <pxr/usd/sdf/path.h>
#include  <pxr/usd/usd/prim.h>

class PrimNode
{
    QString m_name;
    pxr::SdfPath m_path;
    PrimNode *m_parent;
    std::vector<PrimNode *> m_children;
    uint32_t m_row;

public:
    PrimNode(pxr::UsdPrim prim, PrimNode *parent, int row);
    ~PrimNode();

    auto *parent() const { return m_parent; }
    auto &children() { return m_children; }
    auto &name() const { return m_name; }
    auto row() const { return static_cast<int>(m_children.size()); }
    auto path() const { return m_path; }
};