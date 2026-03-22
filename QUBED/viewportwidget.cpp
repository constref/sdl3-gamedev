#include "viewportwidget.h"

#include <QResizeEvent>

#include <logger.h>

void ViewportWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    if (!m_isEngineInit)
    {
        HWND hWnd = reinterpret_cast<HWND>(winId());
        m_engineWorker->start(hWnd, event->size().width(), event->size().height());
        m_isEngineInit = true;
    }
    else
    {
        m_engineWorker->pushEvent(ResizeEvent(0, 0, event->size().width(), event->size().height()));
    }
}

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
}

void ViewportWidget::createWorker(std::unique_ptr<Application> app)
{
    m_engineWorker = std::make_unique<EngineWorker>(std::move(app));
}
