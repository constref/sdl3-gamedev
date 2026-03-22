#pragma once

#include <QWidget>
#include <tooling/engineworker.h>

class ViewportWidget : public QWidget
{
    Q_OBJECT
    bool m_isEngineInit = false;
    std::unique_ptr<EngineWorker> m_engineWorker;
    bool m_mouseGrabbed = false;
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    
public:
    ViewportWidget(QWidget *parent);
    
    void createWorker(std::unique_ptr<Application> app);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    
public slots:
    void onMouseGrabToggle(bool isGrabbed);
};
