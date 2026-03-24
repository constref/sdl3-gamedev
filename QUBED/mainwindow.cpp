#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "viewportwidget.h"
#include <QMainWindow>
#include <QResizeEvent>
#include <QFileDialog>
#include <tooling/usd/usdprocessor.h>

#include "engine.h"
#include "usdstagemodel.h"

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	connect(ui->viewportWidget, &ViewportWidget::viewportResized, this, &MainWindow::onViewportResized);
	connect(ui->viewportWidget, &ViewportWidget::mouseMoved, this, &MainWindow::onViewportMouseMoved, Qt::DirectConnection);
	connect(this, &MainWindow::mouseGrabToggled, ui->viewportWidget, &ViewportWidget::onMouseGrabToggle);
	connect(ui->actionNew_Stage, &QAction::triggered, this, &MainWindow::onNewStage);
	connect(ui->actionOpen_Stage, &QAction::triggered, this, &MainWindow::onOpenStage);
	connect(ui->actionSave_Stage, &QAction::triggered, this, &MainWindow::onSaveStage);
	connect(ui->actionAdd_SubLayer, &QAction::triggered, this, &MainWindow::onAddLayer);
	connect(ui->actionBake_Stage, &QAction::triggered, this, &MainWindow::onBakeStage);

    m_engineWorker = std::make_unique<EngineWorker>(std::move(app));
	m_usdProc = std::make_unique<usd::UsdProcessor>();
}

MainWindow::~MainWindow() { delete ui; }

usd::UsdProcessor &MainWindow::usdProc()
{
	return *m_usdProc;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	uint16_t scancode = event->key();
	m_engineWorker->pushEvent(KeyDown{scancode});
	QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_QuoteLeft)
	{
		m_mouseGrabbed = !m_mouseGrabbed;
		emit mouseGrabToggled(m_mouseGrabbed);
	}
	else
	{
		uint16_t scancode = event->key();
		m_engineWorker->pushEvent(KeyUp{scancode});
	}

	QMainWindow::keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	m_engineWorker->stop();
	QMainWindow::closeEvent(event);
}

void MainWindow::onViewportResized(QSize size)
{
	if (!m_isEngineInit)
	{
		HWND hWnd = reinterpret_cast<HWND>(ui->viewportWidget->winId());
		m_engineWorker->start(hWnd, size.width(), size.height());
		m_isEngineInit = true;
	}
	else
	{
		m_engineWorker->pushEvent(ResizeEvent(0, 0, size.width(), size.height()));
	}
}

void MainWindow::onViewportMouseMoved(int x, int y, int xRel, int yRel)
{
	float sensitivity = 0.5f;
	m_engineWorker->pushEvent(MouseMoveEvent{ x, y, xRel * sensitivity, yRel * sensitivity });
}

void MainWindow::onNewStage()
{
	usdProc().createStage("mynewstage.usda");
	ui->stageView->setModel(new UsdStageModel(this));
}

void MainWindow::onOpenStage()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open Stage"), QDir::homePath(),
													tr("USD Files (*.usd *.usdc *.usda)"));
	if (!filepath.isEmpty())
	{
		usdProc().openStage(filepath.toStdString());
	}
}

void MainWindow::onAddLayer()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Add Layer File"), QDir::homePath(),
	                                                tr("USD Files (*.usd *.usdc *.usda)"));

	if (!filepath.isEmpty())
	{
		usdProc().addLayer(filepath.toStdString());
	}
}

void MainWindow::onSaveStage()
{
	usdProc().saveStage();
}

void MainWindow::onBakeStage() const
{
	m_engineWorker->pushEvent(usd::BakeStageEvent{ .currentProcessor = m_usdProc.get() });
}
