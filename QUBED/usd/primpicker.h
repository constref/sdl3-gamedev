#pragma once

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
	PrimNode *m_selection;

public:
	explicit PrimPicker(usd::UsdProcessor *usdProc, QWidget *parent = nullptr);
	~PrimPicker() override;
	PrimNode *selection() const;

public slots:
	void onSelectionChanged();
};