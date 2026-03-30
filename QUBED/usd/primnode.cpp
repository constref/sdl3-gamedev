#include "primnode.h"

PrimNode::PrimNode(pxr::UsdPrim prim, PrimNode* parent, int row, PrimSpecialization spec)
{
    m_parent = parent;
    m_spec = spec;
    m_path = prim.GetPath();
    m_name = QString::fromStdString(prim.GetName().GetString());
    m_row = row;
}


PrimNode::~PrimNode()
{
    for (auto *child : m_children)
    {
        delete child;
    }
}
