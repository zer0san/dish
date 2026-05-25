#include <QApplication>
#include "terminal/TerminalWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    TerminalWidget terminal;
    terminal.setWindowTitle("dish - Linux Terminal Simulator");
    terminal.resize(800, 500);
    terminal.show();

    return QApplication::exec();
}
