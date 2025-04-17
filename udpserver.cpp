#include <QRegularExpression>

#include "udpserver.h"



UdpServer::UdpServer(QObject *parent)
    : QObject{parent}
{
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::setRetransl(bool retransl)
{
    this->retransl = retransl;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::setRetranslPort(uint16_t port)
{
    this->retransl_port = port;
}
//------------------------------------------------------------------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::onReadyRead()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray data;
        int datagramSize = socket->pendingDatagramSize();
        data.resize(datagramSize);

        qint64 readLen = socket->readDatagram(data.data(), data.size());
        if (readLen == -1) return;

        if(retransl) send(data);
        if(determinePacketType(data)) {
            prosessAscii(data);
        } else
            process(data);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool UdpServer::send(QByteArray data)
{
    clientSocket->writeDatagram(data, QHostAddress::LocalHost, retransl_port);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::process(QByteArray data)
{
    QDataStream in(data);
    in.setVersion(16);
    in.setByteOrder(QDataStream::BigEndian);

    in >> magic >> schema >> message_type;

    switch (message_type)
    {
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
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::prosessAscii(QByteArray data)
{
    qDebug() << "Received UDP message. Type: LOGGED ADIF";
    parseAdifQSO(data);
    qDebug() << "ADIF data: " << data;
    emit loggedADIF();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::parseAdifQSO(const QString &line)
{
    // Разбивать строку на части с помощью '<' и '>'
    QStringList parts = line.split('<', Qt::SkipEmptyParts);

    for (const QString &part : parts) {
        // Найти позицию первого символа ':' для извлечения ключа и значения
        int colonIndex = part.indexOf(':');
        if (colonIndex != -1) {
            // Извлекаем ключ и значение
            QString key = part.left(colonIndex).trimmed();
            QString value = part.mid(colonIndex + 1).trimmed();

            int idx = value.indexOf('>'); // Находим индекс символа '>'

            if (idx != -1) {
               value = value.mid(idx + 1);
            }
            adifData[key] = value;  // Сохраняем ключ-значение в мапу
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool UdpServer::isASCII(char c)
{
    return (c >= 32 && c <= 126); // ASCII characters
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool UdpServer::determinePacketType(const QByteArray& packet)
{
    for (char byte : packet) {
        if (!isASCII(byte)) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------
