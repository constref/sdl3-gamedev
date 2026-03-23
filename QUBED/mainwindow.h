#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <application.h>
#include <memory>

namespace usd
{
class UsdProcessor;
}

class EngineWorker;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Ui::MainWindow *ui;
    QWindow *m_renderWindow;
    bool m_mouseGrabbed = false;
    bool m_isEngineInit = false;
    std::unique_ptr<EngineWorker> m_engineWorker;
    std::unique_ptr<usd::UsdProcessor> m_usdProc;

public:
    MainWindow(std::unique_ptr<Application> app, QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    usd::UsdProcessor &usdProc();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void mouseGrabToggled(bool isGrabbed);

public slots:
    void onViewportResized(QResizeEvent *event);
    void onViewportMouseMoved(int x, int y, int xRel, int yRel);
    void onNewStage();
    void onOpenStage();
    void onSaveStage();
    void onAddLayer();
    void onBakeStage() const;
};
#endif // MAINWINDOW_H
