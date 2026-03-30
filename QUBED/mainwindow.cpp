#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "viewportwidget.h"
#include <SDL3/SDL.h>
#include <QMainWindow>
#include <QResizeEvent>
#include <QFileDialog>
#include <tooling/usd/usdprocessor.h>

#include "usd/primnode.h"
#include "usd/usdstagemodel.h"
#include "usd/treeviewstagelistener.h"
#include "usd/primpicker.h"


std::array<uint32_t, 256> mappedKeys;

MainWindow::MainWindow(std::unique_ptr<Application> app, QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	connect(this, &MainWindow::mouseGrabToggled, ui->viewportWidget, &ViewportWidget::onMouseGrabToggle);
	connect(ui->viewportWidget, &ViewportWidget::viewportResized, this, &MainWindow::onViewportResized);
	connect(ui->viewportWidget, &ViewportWidget::mouseMoved, this, &MainWindow::onViewportMouseMoved, Qt::DirectConnection);
	connect(ui->actionNew_Stage, &QAction::triggered, this, &MainWindow::onNewStage);
	connect(ui->actionOpen_Stage, &QAction::triggered, this, &MainWindow::onOpenStage);
	connect(ui->actionSave_Stage, &QAction::triggered, this, &MainWindow::onSaveStage);
	connect(ui->actionAdd_SubLayer, &QAction::triggered, this, &MainWindow::onAddLayer);
	connect(ui->actionBake_Stage, &QAction::triggered, this, &MainWindow::onBakeStage);
	connect(ui->actionAdd_Mesh, &QAction::triggered, this, &MainWindow::onAddMesh);
	connect(ui->actionAdd_Brush, &QAction::triggered, this, &MainWindow::onAddBrush);
	connect(ui->actionSelect_Brush, &QAction::triggered, this, &MainWindow::onSelectBrush);
	connect(ui->actionPlace_Brush, &QAction::triggered, this, &MainWindow::onPlaceBrush);

    m_engineWorker = std::make_unique<EngineWorker>(std::move(app));
	m_stageListener = std::make_shared<TreeViewStageListener>();
	m_usdProc = std::make_unique<usd::UsdProcessor>(m_stageListener);

	mappedKeys[Qt::Key_W] = SDL_SCANCODE_W;
	mappedKeys[Qt::Key_A] = SDL_SCANCODE_A;
	mappedKeys[Qt::Key_S] = SDL_SCANCODE_S;
	mappedKeys[Qt::Key_D] = SDL_SCANCODE_D;

	qApp->installEventFilter(this);
}

MainWindow::~MainWindow() { delete ui; }

usd::UsdProcessor &MainWindow::usdProc()
{
	return *m_usdProc;
}

uint16_t mapScancode(int qtKey)
{
	if (qtKey < mappedKeys.size())
	{
		return mappedKeys[qtKey];
	}
	return 0;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	m_engineWorker->stop();
	QMainWindow::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		auto *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->isAutoRepeat())
		{
			return true;
		}
		Logger::info(this, std::format("Down {}", keyEvent->key()));
		uint16_t scancode = mapScancode(keyEvent->key());
		m_engineWorker->pushEvent(KeyDown{scancode});
		return true;
	}
	else if (event->type() == QEvent::KeyRelease)
	{
		auto *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_QuoteLeft)
		{
			m_mouseGrabbed = !m_mouseGrabbed;
			emit mouseGrabToggled(m_mouseGrabbed);
		}
		else
		{
			if (keyEvent->isAutoRepeat())
			{
				return true;
			}
			Logger::info(this, std::format("Up {}", keyEvent->key()));
			uint16_t scancode = mapScancode(keyEvent->key());
			m_engineWorker->pushEvent(KeyUp{scancode});
		}
		return true;
	}
	return false;
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

void MainWindow::onPrimPickerAccepted()
{
	disconnect(m_primPicker, &QDialog::accepted, this, &MainWindow::onPrimPickerAccepted);
}

void MainWindow::onNewStage()
{
	QString filepath = QFileDialog::getSaveFileName(this, tr("Open Stage"), QDir::homePath(),
		tr("USD Files (*.usd *.usdc *.usda)"));
	if (!filepath.isEmpty())
	{
		usdProc().createStage(filepath.toStdString());

		if (m_stageModel)
		{
			delete m_stageModel;
		}
		m_stageModel = new UsdStageModel(this);
		m_stageModel->rebuildTree(usdProc().stage());
		connect(m_stageListener.get(), &TreeViewStageListener::primChanged, m_stageModel, &UsdStageModel::onPrimChanged);
		ui->stageView->setModel(m_stageModel);
	}
}

void MainWindow::onOpenStage()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open Stage"), QDir::homePath(),
		tr("USD Files (*.usd *.usdc *.usda)"));
	if (!filepath.isEmpty())
	{
		usdProc().openStage(filepath.toStdString());

		if (m_stageModel)
		{
			delete m_stageModel;
		}
		m_stageModel = new UsdStageModel(this);
		m_stageModel->rebuildTree(usdProc().stage());
		connect(m_stageListener.get(), &TreeViewStageListener::primChanged, m_stageModel, &UsdStageModel::onPrimChanged);
		ui->stageView->setModel(m_stageModel);
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

void MainWindow::onAddMesh()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Reference USD"), QDir::homePath(),
													tr("USD Files (*.usd *.usdc *.usda)"));

	if (!filepath.isEmpty())
	{
		usd::UsdProcessor *assetProc = new usd::UsdProcessor;
		assetProc->openStage(filepath.toStdString());
		m_primPicker = new PrimPicker(assetProc, this);
		m_primPicker->setWindowModality(Qt::WindowModal);
		m_primPicker->exec();

		PrimNode *node = m_primPicker->selection();
		usdProc().addMesh(filepath.toStdString(), node->path());
	}
}

void MainWindow::onAddBrush()
{
	auto selections =  ui->stageView->selectionModel()->selectedIndexes();
	if (!selections.isEmpty())
	{
		PrimNode *node = static_cast<PrimNode *>(selections.first().internalPointer());
		usdProc().addBrush(node->path());
		m_selectedBrush = node;
	}
}

void MainWindow::onSelectBrush()
{
	auto selections =  ui->stageView->selectionModel()->selectedIndexes();
	if (!selections.isEmpty())
	{
		PrimNode *node = static_cast<PrimNode *>(selections.first().internalPointer());
		Logger::info(this, std::format("{} selected", node->path().GetString()));
		m_selectedBrush = node;
	}
}

void MainWindow::onPlaceBrush()
{
	auto selections =  ui->stageView->selectionModel()->selectedIndexes();
	if (!selections.isEmpty())
	{
		usdProc().placeBrush(m_selectedBrush->path());
	}
}
