#include "primpicker.h"
#include "ui_PrimPicker.h"
#include "usdstagemodel.h"

#include <pxr/usd/usd/prim.h>

#include "primnode.h"
#include "tooling/usd/usdprocessor.h"

PrimPicker::PrimPicker(usd::UsdProcessor *usdProc, QWidget *parent) : QDialog(parent), ui(new Ui::PrimPicker)
{
	ui->setupUi(this);
	UsdStageModel *stageModel = new UsdStageModel(this);
	stageModel->rebuildTree(usdProc->stage());
	ui->primTree->setModel(stageModel);
}

PrimPicker::~PrimPicker()
{
	delete ui;
}

QModelIndexList PrimPicker::selections() const
{
	return ui->primTree->selectionModel()->selectedRows();
}
