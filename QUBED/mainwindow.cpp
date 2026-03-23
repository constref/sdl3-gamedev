#include "mainwindow.h"

#include <QMainWindow>
#include <QResizeEvent>
#include "ui_mainwindow.h"
#include "viewportwidget.h"

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->renderWidget->createWorker(std::move(app));
    connect(this, &MainWindow::mouseGrabToggled, ui->renderWidget, &ViewportWidget::onMouseGrabToggle);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_QuoteLeft)
    {
        m_mouseGrabbed = !m_mouseGrabbed;
        emit mouseGrabToggled(m_mouseGrabbed);
    }
    QMainWindow::keyReleaseEvent(event);
}
