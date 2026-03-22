#include "viewportwidget.h"

#include <QResizeEvent>

#include <logger.h>

void ViewportWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    emit viewportResized(event);
}

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
}
