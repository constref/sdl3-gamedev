#include "usdstageitemdelegate.h"

UsdStageItemDelegate::UsdStageItemDelegate()
{
}

UsdStageItemDelegate::~UsdStageItemDelegate()
{
}

QSize UsdStageItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(24);
    return size;
}
