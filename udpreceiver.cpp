#include "udpreceiver.h"

UdpReceiver::UdpReceiver(QObject *parent)
    : QObject{parent}
{
}

bool UdpReceiver::start(uint16_t port) {
    this->port = port;
    socket = new QUdpSocket(this);
    if (!socket->bind(QHostAddress::Any, port)) {
        return false;
    }

    connect(socket, &QUdpSocket::readyRead, this, &UdpReceiver::onReadyRead);

    this->retransmit_port = 2555;
    retransmit_socket = new QUdpSocket(this);
    if (!retransmit_socket->bind(QHostAddress::Any, retransmit_port)) {
        return false;
    }

    return true;
}

void UdpReceiver::onReadyRead() {
    while (socket->hasPendingDatagrams()) {
        QByteArray data;
        int datagramSize = socket->pendingDatagramSize();
        data.resize(datagramSize);

        qint64 readLen = socket->readDatagram(data.data(), data.size());
        if (readLen == -1) return;
        process(data);
        retransmit_socket->writeDatagram(data.data(), QHostAddress::Any, retransmit_port);
    }
}

void UdpReceiver::process(QByteArray data) {
    QDataStream in(data);
    in.setVersion(16);
    in.setByteOrder(QDataStream::BigEndian);

    in >> magic >> schema >> message_type;

    switch (message_type) {
        case Heartbeat:
            in >> id >> max_schema_number >> version >> revision;
            emit heartbeat();
        break;

        case QSOLogged:
            qDebug() << "Received UDP message. Type: LOGGED, Payload size (bytes):" << data.size();
            in >> id >> time_off >> dx_call >> dx_grid >> tx_frequency_hz >> mode >> report_sent >> report_received >> tx_power >> comments >> name >> time_on >> operator_call >> my_call >> my_grid >> exchange_sent >> exchange_rcvd >> propmode;
            qDebug() << "Structed data [id, time_off, dx_call, dx_grid, tx_frequency_hz, mode, report_sent, report_received, tx_power, comments, name, time_on, operator_call, my_call, my_grid, exchange_sent, exchange_rcvd, propmode]:" << id << time_off << dx_call << dx_grid << tx_frequency_hz << mode << report_sent << report_received << tx_power << comments << name << time_on << operator_call << my_call << my_grid << exchange_sent << exchange_rcvd << propmode;
            emit logged();
        break;
    }
}
