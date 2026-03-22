#include "editorlauncher.h"
#include <tooling/engineworker.h>

#include "mainwindow.h"
#include <QApplication>

int EditorLauncher::exec(std::unique_ptr<Application> app, int argc, char* argv[])
{
    EngineWorker worker(std::move(app));
    worker.start();
		
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
		
    return QCoreApplication::exec();
}
