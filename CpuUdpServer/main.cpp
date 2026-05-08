#include <QApplication>
#include "cpumonitorwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    quint16 port = 9999;
    if (argc > 1) {
        port = QString(argv[1]).toUShort();
    }

    CpuMonitorWindow w(port);
    w.show();

    return a.exec();
}
