#include "primnode.h"
#include  <pxr/usd/usd/prim.h>

PrimNode::PrimNode(const std::string &name, PrimNode *parent)
{
    m_parent = parent;
    m_path = prim.GetPath();
    m_name = QString::fromStdString(name);
}
