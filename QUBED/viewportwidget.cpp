#include "viewportwidget.h"

#include <QResizeEvent>
#include <QTimer>

ViewportWidget::ViewportWidget(QWidget *parent) : QWidget(parent)
{
	m_resizeTimer = new QTimer();
	connect(m_resizeTimer, &QTimer::timeout, this, &ViewportWidget::performResize);
}

void ViewportWidget::performResize()
{
	m_resizeTimer->stop();
	emit viewportResized(size());
}

void ViewportWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	m_resizeTimer->start(500);
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
			emit mouseMoved(pos.x(), pos.y(), delta.x(), delta.y());
			QCursor::setPos(globalCenter);
		}
	}
	QWidget::mouseMoveEvent(event);
}

void ViewportWidget::onMouseGrabToggle(bool isGrabbed)
{
	m_mouseGrabbed = isGrabbed;
	setMouseTracking(m_mouseGrabbed);
	if (isGrabbed)
	{
		grabMouse();
		setCursor(Qt::BlankCursor);
	} else
	{
		releaseMouse();
		setCursor(Qt::ArrowCursor);
	}
}
