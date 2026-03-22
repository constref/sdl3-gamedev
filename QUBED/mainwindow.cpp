#include "mainwindow.h"

#include <QMainWindow>
#include <QResizeEvent>
#include "ui_mainwindow.h"
#include "viewportwidget.h"

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->renderWidget->createWorker(std::move(app));
}

MainWindow::~MainWindow()
{
    delete ui;
}