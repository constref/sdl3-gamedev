#include "primpicker.h"
#include "ui_PrimPicker.h"
#include "usdstagemodel.h"

#include <pxr/usd/usd/prim.h>

#include "primnode.h"
#include "tooling/usd/usdprocessor.h"

PrimPicker::PrimPicker(usd::UsdProcessor *usdProc, QWidget *parent) : QDialog(parent), ui(new Ui::PrimPicker)
{
	m_selection = nullptr;
	ui->setupUi(this);

	UsdStageModel *stageModel = new UsdStageModel(this);
	stageModel->rebuildTree(usdProc->stage());
	ui->primTree->setModel(stageModel);

	connect(ui->primTree->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &PrimPicker::onSelectionChanged);
}

PrimPicker::~PrimPicker()
{
	delete ui;
}

PrimNode * PrimPicker::selection() const
{
	return m_selection;
}

void PrimPicker::onSelectionChanged()
{
	auto selModel = ui->primTree->selectionModel();
	if (selModel->hasSelection())
	{
		for (QModelIndex idx : selModel->selectedRows())
		{
			PrimNode *node = static_cast<PrimNode *>(idx.internalPointer());
			m_selection = node;
		}
	}
}
