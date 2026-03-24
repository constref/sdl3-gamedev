#include "editorlauncher.h"
#include <tooling/engineworker.h>

#include "mainwindow.h"
#include <QApplication>

int EditorLauncher::exec(std::unique_ptr<Application> app, int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w(std::move(app));
    w.showMaximized();
		
    return QCoreApplication::exec();
}
