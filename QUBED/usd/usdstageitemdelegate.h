#pragma once

#include <QStyledItemDelegate>

class UsdStageItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit UsdStageItemDelegate();
    ~UsdStageItemDelegate() override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};