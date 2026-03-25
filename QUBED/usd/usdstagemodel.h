#pragma once

#include <QAbstractItemModel>

namespace usd
{
class UsdProcessor;
}

class PrimNode;

class UsdStageModel : public QAbstractItemModel
{
	PrimNode *rootPrim;

public:
	UsdStageModel(usd::UsdProcessor &usdProc, QObject *parent);
	QModelIndex index(int row, int column, const QModelIndex &parent) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};