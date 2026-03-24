#include "usdstagemodel.h"

UsdStageModel::UsdStageModel(QObject *parent) : QAbstractItemModel(parent)
{
}

QModelIndex UsdStageModel::index(int row, int column, const QModelIndex &parent) const
{
	return createIndex(row, column, nullptr);
}

QModelIndex UsdStageModel::parent(const QModelIndex &child) const
{
	return QModelIndex();
}

int UsdStageModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
	{
		return 1;
	}
	return 0;
}

int UsdStageModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

QVariant UsdStageModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
			case 0:
			{
				return QString("Library");
			}
			case 1:
			{
				return QString("Scope");
			}
			case 3:
			{
				return QString("");
			}
			default:
			{
				return QVariant();
			}
		}
	}
	else
	{
		return QVariant();
	}
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
				return QString("Scope");
			}
			case 2:
			{
				return QString("Kind");
			}
		}
	}
	return QAbstractItemModel::headerData(section, orientation, role);
}
