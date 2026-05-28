#include <QApplication>
#include "user/userSystem.hpp"
#include "fs/fs.hpp"
#include "terminal/TerminalWidget.h"
#include "init.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QString diskPath = Init::initDataPath();

    UserSystem userSystem;

    FileSystem fileSystem;
    Init::initFileSystem(fileSystem, diskPath);
    Init::initStandardDirs(fileSystem);

    TerminalWidget terminal(&userSystem, &fileSystem, diskPath);
    terminal.setWindowTitle("dish - Linux Terminal Simulator");
    terminal.resize(800, 500);
    terminal.show();

    int ret = QApplication::exec();

    if (fileSystem.isMounted()) {
        fileSystem.unmount();
    }

    return ret;
}