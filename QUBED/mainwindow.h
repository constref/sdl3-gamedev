#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <application.h>
#include <memory>

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

public:
    MainWindow(std::unique_ptr<Application> app, QWidget *parent = nullptr);
    ~MainWindow() override;
};
#endif // MAINWINDOW_H
