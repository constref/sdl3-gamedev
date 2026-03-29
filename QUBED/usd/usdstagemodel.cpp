#include "usdstagemodel.h"
#include <QHeaderView>
#include "primnode.h"
#include <tooling/usd/usdprocessor.h>
#include <pxr/usd/usd/notice.h>

using namespace pxr;

UsdStageModel::UsdStageModel(QObject *parent) : QAbstractItemModel(parent)
{
}

QModelIndex UsdStageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        // parent is qt tree-root
        return createIndex(row, column, m_rootPrim->children()[row]);
    }
    auto *parentNode = static_cast<PrimNode*>(parent.internalPointer());
    auto *node = parentNode->children()[row];
    return createIndex(row, column, node);
}

QModelIndex UsdStageModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        return {};
    }

    PrimNode *childNode = static_cast<PrimNode *>(child.internalPointer());
    PrimNode *parentNode = childNode->parent();
    if (!parentNode || parentNode == m_rootPrim)
    {
        return {};
    }
    return createIndex(parentNode->row(), 0, parentNode);
}

int UsdStageModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return m_rootPrim->children().size();
    }
    PrimNode *parentNode = static_cast<PrimNode*>(parent.internalPointer());
    return parentNode->children().size();
}

int UsdStageModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant UsdStageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    if (role == Qt::DisplayRole)
    {
        PrimNode *node = static_cast<PrimNode*>(index.internalPointer());
        switch (index.column())
        {
            case 0:
            {
                return node->name();
            }
            case 1:
            {
                return QString("");
            }
            default:
            {
                return {};
            }
        }
    }
    return {};
}

QVariant UsdStageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
                case 0:
                {
                    return QString("Name");
                }
                case 1:
                {
                    return QString("Kind");
                }
            }
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void UsdStageModel::walkStage(UsdPrim prim, PrimNode *parent)
{
    int rowNum = 0;
    for (auto child : prim.GetChildren())
    {
        auto *node = new PrimNode(child, parent, rowNum);
        m_primMap[child.GetPath()] = node;
        parent->children().push_back(node);
        rowNum++;

        walkStage(child, node);
    }
}

void UsdStageModel::rebuildTree(UsdStageRefPtr stage)
{
    UsdPrim root = stage->GetPseudoRoot();
    m_rootPrim = new PrimNode(root, nullptr, 0);
    m_primMap[root.GetPath()] = m_rootPrim;

    walkStage(root, m_rootPrim);
}

void UsdStageModel::onPrimChanged(SdfPath path, UsdStageRefPtr stage)
{
    auto itr = m_primMap.find(path);
    Logger::info(this, std::format("{} path changed", path.GetString()));
    if (itr != m_primMap.end())
    {
        UsdPrim prim = stage->GetPrimAtPath(path);
        PrimNode *node = itr->second;
    }
    else
    {
        // new prim, get parent and add it
        auto itr = m_primMap.find(path.GetParentPath());
        if (itr != m_primMap.end())
        {
            PrimNode *parentNode = itr->second;
            QModelIndex parentIndex = parentNode == m_rootPrim
                ? QModelIndex()
                : createIndex(parentNode->row(), 0, parentNode);

            beginInsertRows(parentIndex, parentNode->children().size(), parentNode->children().size());

            UsdPrim prim = stage->GetPrimAtPath(path);
            PrimNode *node = new PrimNode(prim, parentNode, parentNode->children().size());
            parentNode->children().push_back(node);

            endInsertRows();
            m_primMap[path] = node;
        }
    }
}
