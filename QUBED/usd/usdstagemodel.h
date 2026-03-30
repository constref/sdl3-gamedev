#pragma once

#include <unordered_map>
#include <QAbstractItemModel>
#include <tooling/usd.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/tf/hash.h>


namespace usd
{
class UsdProcessor;
}

class PrimNode;

class UsdStageModel : public QAbstractItemModel
{
	Q_OBJECT
	PrimNode *m_rootPrim;
	std::unordered_map<pxr::SdfPath, PrimNode *, pxr::TfHash> m_primMap;

	void walkStage(pxr::UsdPrim prim, PrimNode *parent);
public:
	UsdStageModel(QObject *parent = nullptr);
	~UsdStageModel() override;

	QModelIndex index(int row, int column, const QModelIndex &parent) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	void rebuildTree(pxr::UsdStageRefPtr stage);

public slots:
	void onPrimChanged(pxr::SdfPath path, pxr::UsdStageRefPtr stage);

};
