#include "editorlauncher.h"
#include <QApplication>
#include <tooling/engineworker.h>
#include "mainwindow.h"

int EditorLauncher::exec(std::unique_ptr<Application> app, int argc, char* argv[])
{
    QApplication application(argc, argv);
    MainWindow win(std::move(app));
    win.showMaximized();

    return QCoreApplication::exec();
}
