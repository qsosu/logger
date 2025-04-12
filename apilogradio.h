#ifndef APILOGRADIO_H
#define APILOGRADIO_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUrlQuery>
#include <QtNetwork>
#include <QMessageBox>


class APILogRadio : public QObject
{
    Q_OBJECT
public:
    explicit APILogRadio(QString APILogRadioAccessToken, QObject *parent = nullptr);
    void getToken();
    bool checkToken();
    void SendQso(QVariantList data);

    QNetworkAccessManager m_manager;
    QString APILogRadioAccessToken;

private:
    QString XApplicationKey = "f23005c5-681e-43fb-96fa-6e80c89cbb9c";

signals:
    void received(QString, QString, QString, QString, QString, QString);
    void checked(int, QString);
    void QSOStatus(int);
    void synced(int);
};

#endif // APILOGRADIO_H
