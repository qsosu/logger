#include <QRegularExpression>
#include "udpserver.h"
#include <QTimer>


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

bool UdpServer::start(uint16_t port)
{
    this->port = port;
    qDebug() << "[UDP] Starting server on port" << port;
    stop(); // гарантированно чистый старт

    socket = new QUdpSocket(this);
    clientSocket = new QUdpSocket(this);

    if (!socket->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qCritical() << "[UDP] Bind failed:" << socket->errorString();
        socket->deleteLater();
        socket = nullptr;
        return false;
    }
    connect(socket, &QUdpSocket::readyRead, this, &UdpServer::onReadyRead);
    connect(socket, &QUdpSocket::errorOccurred, this, &UdpServer::onSocketError);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::stop()
{
    if (socket) {
        disconnect(socket, nullptr, this, nullptr);
        socket->close();
        socket->deleteLater();
        socket = nullptr;
    }

    if (clientSocket) {
        clientSocket->close();
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }
    port = 0;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::onSocketError(QAbstractSocket::SocketError error)
{
    qCritical() << "[UDP] Socket error:" << error << socket->errorString();
    // Перезапуск с задержкой
    QTimer::singleShot(1000, this, &UdpServer::restartSocket);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::restartSocket()
{
    qWarning() << "[UDP] Restarting socket on port" << port;
    stop();
    start(port);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UdpServer::onReadyRead()
{
    if (!socket) return;

    while (socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(int(socket->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort;
        qint64 readLen = socket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        if (readLen < 0) {
            qCritical() << "[UDP] readDatagram failed:" << socket->errorString();
            restartSocket();
            return;
        }
        if (retransl) send(data);

        if (determinePacketType(data))
            prosessAscii(data);
        else
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

    case Status:
        in >> id;
        in >> tx_frequency_hz;
        in >> mode;
        in >> dx_call;
        in >> dx_report;
        in >> tx_mode;
        in >> tx_enabled;
        in >> transmitting;
        in >> rx_tx_period;
        in >> tx_df;
        in >> my_call;
        in >> my_grid;
        in >> dx_grid;
        in >> tx_df_auto;
        in >> flags;
        emit status();
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




