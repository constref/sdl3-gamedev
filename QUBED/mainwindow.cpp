#include "mainwindow.h"

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "viewportwidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //HWND hWnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
}

MainWindow::~MainWindow()
{
    delete ui;
}
