#include "mainwindow.h"

#include <QMainWindow>
#include <QResizeEvent>

#include "ui_mainwindow.h"

#include "viewportwidget.h"

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    m_engineWorker = std::make_unique<EngineWorker>(std::move(app));
    connect(ui->renderWidget, &ViewportWidget::viewportResized, this, &MainWindow::onViewportResized);
    //worker.start();
    //HWND hWnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onViewportResized(QResizeEvent *event)
{
    if (!m_isEngineInit)
    {
        HWND hWnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
        m_engineWorker->start(hWnd, event->size().width(), event->size().height());
        m_isEngineInit = true;
    }
    else
    {
        m_engineWorker->pushEvent(ResizeEvent(0, 0, event->size().width(), event->size().height()));
    }
}
