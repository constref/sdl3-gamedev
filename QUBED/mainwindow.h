#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <application.h>
#include <memory>

#include <tooling/engineworker.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QWindow *m_renderWindow;
    std::unique_ptr<EngineWorker> m_engineWorker;
    bool m_isEngineInit = false;

public:
    MainWindow(std::unique_ptr<Application> app, QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onViewportResized(QResizeEvent *event);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
