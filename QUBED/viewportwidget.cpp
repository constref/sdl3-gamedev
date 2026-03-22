#include "viewportwidget.h"

void ViewportWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

ViewportWidget::ViewportWidget(QWidget* parent) : QWidget(parent)
{
}
