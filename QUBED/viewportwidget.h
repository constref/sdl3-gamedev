#pragma once

#include <QWidget>
#include <tooling/engineworker.h>

class ViewportWidget : public QWidget
{
    Q_OBJECT
    bool m_isEngineInit = false;
    std::unique_ptr<EngineWorker> m_engineWorker;
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    
public:
    ViewportWidget(QWidget *parent);
    
    void createWorker(std::unique_ptr<Application> app);
};
