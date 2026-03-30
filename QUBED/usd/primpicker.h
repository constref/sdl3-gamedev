#pragma once

#include <qabstractitemmodel.h>
#include <tooling/usd.h>

#include <QDialog>

class PrimNode;

namespace usd { class UsdProcessor; }

QT_BEGIN_NAMESPACE

namespace Ui
{
class PrimPicker;
}

QT_END_NAMESPACE

class PrimPicker : public QDialog
{
	Q_OBJECT
	Ui::PrimPicker *ui;

public:
	explicit PrimPicker(usd::UsdProcessor *usdProc, QWidget *parent = nullptr);
	~PrimPicker() override;
	QModelIndexList selections() const;
};