#pragma once

#include <QWidget>
#include <tooling/engineworker.h>

class ViewportWidget : public QWidget
{
    Q_OBJECT
    bool m_isEngineInit = false;
    bool m_mouseGrabbed = false;
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public:
    ViewportWidget(QWidget *parent);

public slots:
    void onMouseGrabToggle(bool isGrabbed);

signals:
    void viewportResized(QResizeEvent *event);
    void mouseMoved(int x, int y, int xRel, int yRel);
};
