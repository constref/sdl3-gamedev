#include "viewportwidget.h"

#include <QResizeEvent>
#include <tooling/platformevents.h>
#include <application.h>

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
}

void ViewportWidget::resizeEvent(QResizeEvent *event)
{
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
    QWidget::resizeEvent(event);
}

void ViewportWidget::createWorker(std::unique_ptr<Application> app)
{
    m_engineWorker = std::make_unique<EngineWorker>(std::move(app));
}

void ViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseGrabbed)
    {
        QPoint globalCenter = mapToGlobal(rect().center());
        QPoint pos = QCursor::pos();
        
        if (pos != globalCenter)
        {
            QPoint delta = pos - globalCenter;
            QCursor::setPos(globalCenter);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void ViewportWidget::keyPressEvent(QKeyEvent *event)
{
    uint16_t scancode = event->key();
    m_engineWorker->pushEvent(KeyDown{scancode});
    QWidget::keyPressEvent(event);
}

void ViewportWidget::keyReleaseEvent(QKeyEvent *event)
{
    uint16_t scancode = event->key();
    m_engineWorker->pushEvent(KeyUp{scancode});
    QWidget::keyReleaseEvent(event);
}

void ViewportWidget::onMouseGrabToggle(bool isGrabbed)
{
    m_mouseGrabbed = isGrabbed;
    if (isGrabbed)
    {
        grabMouse();
        setCursor(Qt::BlankCursor);
    }
    else
    {
       releaseMouse();
        setCursor(Qt::ArrowCursor);
    }
}
