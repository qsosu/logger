#include "udpserver.h"

UdpServer::UdpServer(QObject *parent)
    : QObject{parent}
{
}

void UdpServer::setRetransl(bool retransl)
{
    this->retransl = retransl;
}

void UdpServer::setRetranslPort(uint16_t port)
{
    this->retransl_port = port;
}

bool UdpServer::start(uint16_t port) {
    this->port = port;
    clientSocket = new QUdpSocket(this);
    socket = new QUdpSocket(this);
    if (!socket->bind(QHostAddress::Any, port)) {
        return false;
    }
    connect(socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);
    return true;
}

void UdpServer::onReadyRead() {
    while (socket->hasPendingDatagrams()) {
        QByteArray data;
        int datagramSize = socket->pendingDatagramSize();
        data.resize(datagramSize);

        qint64 readLen = socket->readDatagram(data.data(), data.size());
        if (readLen == -1) return;

        if(retransl) send(data);
        process(data);
    }
}

bool UdpServer::send(QByteArray data)
{
    clientSocket->writeDatagram(data, QHostAddress::LocalHost, retransl_port);
    return true;
}

void UdpServer::process(QByteArray data) {
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
