#ifndef CPUMONITORWINDOW_H
#define CPUMONITORWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include "qcustomplot.h"

class CpuMonitorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit CpuMonitorWindow(quint16 port, QWidget *parent = nullptr);
    ~CpuMonitorWindow();

private slots:
    void processDatagrams();

private:
    QCustomPlot *plot;
    QUdpSocket *udpSocket;
    void setupPlot(); // Вспомогательный метод для настройки визуала
};

#endif // CPUMONITORWINDOW_H
