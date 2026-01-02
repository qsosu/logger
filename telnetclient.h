#ifndef TELNETCLIENT_H
#define TELNETCLIENT_H

#include <QObject>
#include <QDialog>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlQuery>
#include "httpapi.h"



class TelnetClient : public QObject
{
    Q_OBJECT

public:
    TelnetClient(QSqlDatabase db, const QString &host, quint16 port, QObject *parent = nullptr);
    void disconnectFromServer();
    bool TelnetConnected = false;

private slots:
    void connectToServer();
    void onReadyRead();

signals:
    void newsMessageReceived();
    void chatMessageReceived(const Message &msg);
    void newSpotReseived(); // Сигнал при получении спота
    void newChatReseived();

private:
    QTcpSocket *socket;
    QString host;
    int port;
    QSqlDatabase db;

    void insertSpot(const QJsonObject &data);
    void insertNews(const QJsonObject &data);
};

#endif // TELNETCLIENT_H
