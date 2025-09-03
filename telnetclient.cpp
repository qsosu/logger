#include "telnetclient.h"

#include <QTcpSocket>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QTimer>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>


TelnetClient::TelnetClient(QSqlDatabase db, const QString &host, quint16 port, QObject *parent)
    : QObject(parent), host(host), port(port)
{
    this->db = db;
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, []() {
        qDebug() << "Connected to Telnet server.";
    });

    connect(socket, &QTcpSocket::readyRead, this, &TelnetClient::onReadyRead);

    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "Telnet client disconnected. Reconnecting in 10 seconds...";
        QTimer::singleShot(10000, this, &TelnetClient::connectToServer);
    });

    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, [](QAbstractSocket::SocketError err) {
        qDebug() << "Error connecting to telnet server:" << err;
    });

    connectToServer();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void TelnetClient::connectToServer()
{
    if (socket->state() != QAbstractSocket::ConnectedState &&
            socket->state() != QAbstractSocket::ConnectingState)
    {
        qDebug() << "Connecting to" << host << ":" << port;
        socket->abort();
        socket->connectToHost(host, port);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void TelnetClient::disconnectFromServer()
{
    if (socket->state() == QAbstractSocket::ConnectedState ||
        socket->state() == QAbstractSocket::ConnectingState)
    {
        qDebug() << "Disconnecting from" << host << ":" << port;
        socket->disconnectFromHost();

        if (socket->state() != QAbstractSocket::UnconnectedState) {
            // Если сервер не закрыл соединение сразу — принудительно
            socket->abort();
        }
    }
    else {
        qDebug() << "Already disconnected";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void TelnetClient::onReadyRead()
{
    static QByteArray buffer; // сохраняем остаток между вызовами
    buffer.append(socket->readAll());

    // Фильтр Telnet-команд IAC (255)
    QByteArray cleanData;
    for (int i = 0; i < buffer.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(buffer[i]);
        if (c == 255) { i += 2; continue; }
        cleanData.append(buffer[i]);
    }
    buffer = cleanData;

    int start = buffer.indexOf('{');
    while (start != -1) {
        int balance = 0;
        int end = -1;
        for (int i = start; i < buffer.size(); ++i) {
            if (buffer[i] == '{') balance++;
            if (buffer[i] == '}') balance--;
            if (balance == 0) { end = i; break; }
        }

        if (end != -1) {
            QByteArray jsonData = buffer.mid(start, end - start + 1);
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                QString event = obj.value("event").toString();
                QJsonObject data = obj.value("data").toObject();
                QString eventAt = obj.value("event_at").toString();

                // Исправляем опечатку callsing → callsign
                if (data.contains("callsing") && !data.contains("callsign")) {
                    data.insert("callsign", data.value("callsing").toString());
                    data.remove("callsing");
                }

                // Добавляем event_at прямо в data
                data.insert("event_at", eventAt);

                if (event == "spot") {
                    // Берем dxcc из data, если оно есть
                    QString country      = data.value("country").toString();
                    QString countryCode  = data.value("country_code").toString();
                    QString continent    = data.value("continent").toString();
                    QString cqz          = data.value("cqz").toString();
                    QString ituz         = data.value("ituz").toString();

                    // Добавляем разобранный dxcc
                    data.insert("dxcc_country", country);
                    data.insert("dxcc_country_code", countryCode);
                    data.insert("dxcc_continent", continent);
                    data.insert("dxcc_cqz", cqz);
                    data.insert("dxcc_ituz", ituz);

                    insertSpot(data);
                    emit newSpotReseived();
                    QApplication::processEvents();
                }
                else if (event == "news")
                {
                    insertNews(data);
                    emit newsMessageReceived();
                    QApplication::processEvents();
                }
                else if (event == "message")
                {
                    Message msg;
                    msg.id = data.value("id").toInt();
                    msg.chatId = data.value("id_chat").toInt();
                    msg.sender = data.value("sender").toString();
                    msg.text = data.value("text").toString();
                    msg.textColor = data.value("text_color").toString();
                    msg.backgroundColor = data.value("background_color").toString();
                    msg.sentAt = data.value("sent_at").toString();

                    emit chatMessageReceived(msg);
                    emit newChatReseived();
                    qDebug() << "Receive chat message." << msg.text;
                    QApplication::processEvents();
                }
            }
            buffer.remove(0, end + 1); // удаляем обработанный JSON
            start = buffer.indexOf('{');
        } else break; // полный JSON пока не пришёл
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void TelnetClient::insertSpot(const QJsonObject &data)
{
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO spots (
            spotter, callsign, frequency, message, message_raw,
            mode, submode, band, wave_type,
            event_at,
            dxcc_country, dxcc_country_code, dxcc_continent, dxcc_cqz, dxcc_ituz
        )
        VALUES (
            :spotter, :callsign, :frequency, :message, :message_raw,
            :mode, :submode, :band, :wave_type,
            :event_at,
            :dxcc_country, :dxcc_country_code, :dxcc_continent, :dxcc_cqz, :dxcc_ituz
        )
    )");

    query.bindValue(":spotter", data.value("spotter").toString());
    query.bindValue(":callsign", data.value("callsign").toString());   // исправленный callsign
    query.bindValue(":frequency", data.value("frequency").toDouble());
    query.bindValue(":message", data.value("message").toString());
    query.bindValue(":message_raw", data.value("message_raw").toString());
    query.bindValue(":mode", data.value("mode").toString());
    query.bindValue(":submode", data.value("submode").toString());
    query.bindValue(":band", data.value("band").toString());
    query.bindValue(":wave_type", data.value("wave_type").toString());

    // новые поля
    query.bindValue(":event_at", data.value("event_at").toString());
    query.bindValue(":dxcc_country", data.value("dxcc_country").toString());
    query.bindValue(":dxcc_country_code", data.value("dxcc_country_code").toString());
    query.bindValue(":dxcc_continent", data.value("dxcc_continent").toString());
    query.bindValue(":dxcc_cqz", data.value("dxcc_cqz").toString());
    query.bindValue(":dxcc_ituz", data.value("dxcc_ituz").toString());

    if (!query.exec()) {
        qWarning() << "insertSpot error:" << query.lastError().text();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void TelnetClient::insertNews(const QJsonObject &data)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO news (date, title, text) VALUES (?, ?, ?)");
    query.addBindValue(data.value("date").toString());
    query.addBindValue(data.value("title").toString());
    query.addBindValue(data.value("text").toString());

    if (!query.exec()) {
        qWarning() << "Failed to insert news:" << query.lastError().text();
    } else {
        qDebug() << "News saved to DB.";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
