#include "usdstagemodel.h"
#include "primnode.h"

#include <tooling/usd/usdprocessor.h>
using namespace pxr;

void UsdStageModel::walkStage(UsdPrim prim, PrimNode *parent)
{
	int rowNum = 0;
	for (auto child : prim.GetChildren())
	{
		auto *node = new PrimNode(child, parent, rowNum);
		parent->children().push_back(node);
		rowNum++;

		walkStage(child, node);
	}
}

UsdStageModel::UsdStageModel(usd::UsdProcessor &usdProc, QObject *parent) : QAbstractItemModel(parent)
{
	auto stage = usdProc.stage();
	UsdPrim root = stage->GetPseudoRoot();
	m_rootPrim = new PrimNode(root, nullptr, 0);
	walkStage(root, m_rootPrim);
}

QModelIndex UsdStageModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!parent.isValid())
	{
		// parent is qt tree-root
		return createIndex(row, column, m_rootPrim->children()[row]);
	}
	else
	{
		auto *parentNode = static_cast<PrimNode *>(parent.internalPointer());
		auto *node = parentNode->children()[row];
		return createIndex(row, column, node);
	}
}

QModelIndex UsdStageModel::parent(const QModelIndex &child) const
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

int UsdStageModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
	{
		return m_rootPrim->children().size();
	}
	PrimNode *parentNode = static_cast<PrimNode *>(parent.internalPointer());
	return parentNode->children().size();
}

int UsdStageModel::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QVariant UsdStageModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return {};
	}

	if (role == Qt::DisplayRole)
	{
		PrimNode *node = static_cast<PrimNode *>(index.internalPointer());
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
	if (role == Qt::ItemDataRole::DisplayRole && orientation == Qt::Horizontal)
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
	return QAbstractItemModel::headerData(section, orientation, role);
}
