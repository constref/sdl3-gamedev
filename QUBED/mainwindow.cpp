#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "viewportwidget.h"
#include <QMainWindow>
#include <QResizeEvent>

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->renderWidget->createWorker(std::move(app));
	connect(this, &MainWindow::mouseGrabToggled, ui->renderWidget,
	        &ViewportWidget::onMouseGrabToggle);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_QuoteLeft)
	{
		m_mouseGrabbed = !m_mouseGrabbed;
		emit mouseGrabToggled(m_mouseGrabbed);
	}
	QMainWindow::keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	ui->renderWidget->shutdownWorker();
	QMainWindow::closeEvent(event);
}
