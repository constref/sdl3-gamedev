#pragma once

#include <QWidget>

class ViewportWidget : public QWidget
{
protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Q_OBJECT

public:
    ViewportWidget(QWidget *parent);
    
signals:
    void viewportResized(QResizeEvent *event);
};
