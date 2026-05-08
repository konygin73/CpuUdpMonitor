#include "cpumonitorwindow.h"
#include <QNetworkDatagram>
#include <QDateTime>
#include <QMessageBox>

CpuMonitorWindow::CpuMonitorWindow(quint16 port, QWidget *parent)
    : QMainWindow(parent)
{
    plot = new QCustomPlot(this);
    setCentralWidget(plot);
    resize(800, 400);
    setWindowTitle(QString("UDP CPU Monitor | Port: %1").arg(port));

    setupPlot();

    udpSocket = new QUdpSocket(this);
    if (!udpSocket->bind(QHostAddress::Any, port)) {
        QMessageBox::critical(this, "Error", "Could not bind port");
        exit(1);
    }

    connect(udpSocket, &QUdpSocket::readyRead, this, &CpuMonitorWindow::processDatagrams);
}

CpuMonitorWindow::~CpuMonitorWindow() {
}

void CpuMonitorWindow::setupPlot() {
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeFormat("mm:ss");
    plot->xAxis->setTicker(dateTimeTicker);
    plot->xAxis->setLabel("Время (сек)");
    plot->yAxis->setLabel("Загрузка (%)");
    plot->yAxis->setRange(0, 105);
    plot->legend->setVisible(true);
    plot->setAntialiasedElements(QCP::aeAll);
}

void CpuMonitorWindow::processDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QByteArray data = datagram.data();

        const double *usage = reinterpret_cast<const double*>(data.constData());
        int count = data.size() / sizeof(double);

        if (plot->graphCount() != count) {
            plot->clearGraphs();
            for (int i = 0; i < count; ++i) {
                QCPGraph *g = plot->addGraph();
                g->setName((i == 0) ? "Total" : QString("Core %1").arg(i-1));
                QColor color = QColor::fromHsv(i * 255 / qMax(1, count), 200, 200);
                g->setPen(QPen(color, 2));
            }
        }

        double now = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
        for (int i = 0; i < count; ++i) {
            plot->graph(i)->addData(now, usage[i]);
        }

        plot->xAxis->setRange(now, 30, Qt::AlignRight);
        plot->replot();
    }
}
