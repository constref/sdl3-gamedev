#pragma once

#include <QAbstractItemModel>
#include <tooling/usd.h>
#include <pxr/usd/usd/prim.h>


namespace usd
{
class UsdProcessor;
}

class PrimNode;

class UsdStageModel : public QAbstractItemModel
{
	PrimNode *m_rootPrim;

	void walkStage(pxr::UsdPrim prim, PrimNode *parent);
public:

	UsdStageModel(usd::UsdProcessor &usdProc, QObject *parent);
	QModelIndex index(int row, int column, const QModelIndex &parent) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
