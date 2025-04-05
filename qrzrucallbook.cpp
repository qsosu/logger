#include "qrzrucallbook.h"
#include <QEventLoop>
#include <QXmlStreamReader>
#include <QTimer>

QrzruCallbook::QrzruCallbook(QString username, QString password, QObject *parent)
    : QObject{parent}
{
    this->username = username;
    this->password = password;
    session_id = "";
    session_time = QDateTime::fromString("01-01-1970 00:00:00", "MM-dd-yyyy hh:mm:ss");
}

void QrzruCallbook::GetApiSession() {
    QByteArray data = Request(QString("https://api.qrz.ru/login?u=%1&p=%2&agent=MQL").arg(username, password));
    session_id = getTagValue(data, "session_id");
    session_time = QDateTime::currentDateTime();
}

QStringList QrzruCallbook::Get(QString call) {
    QString ncall = call.left(call.indexOf('/'));
    QStringList ret;
    if (session_time.secsTo(QDateTime::currentDateTime()) > 3600) {
        GetApiSession();
    }

    QByteArray data = Request(QString("https://api.qrz.ru/callsign?id=%1&callsign=%2").arg(session_id, ncall));
    QString name = getTagValue(data, "name");
    QString city = getTagValue(data, "city");
    QString qthloc = getTagValue(data, "qthloc");
    QString cnty = getTagValue(data, "state");
    QString photo = getTagValue(data, "file");
    ret << name << city << qthloc << cnty << photo;
    return ret;
}

void QrzruCallbook::LoadPhoto(QString imageUrl)
{
    QNetworkAccessManager manager;
    QEventLoop eventLoop;
    QTimer tot;

    QObject::connect(&tot, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(&manager, &QNetworkAccessManager::finished,
                     &eventLoop, &QEventLoop::quit);

    QNetworkRequest request((QUrl(imageUrl)));
    request.setRawHeader("Content-Type", "image");
    QNetworkReply *reply = manager.get(request);

    tot.start(1000);
    eventLoop.exec();

    if (tot.isActive()) {
        tot.stop();
        if (reply->error() != QNetworkReply::NoError) {
            switch (reply->error()) {
                case QNetworkReply::ContentNotFoundError:
                    qDebug() << "ContentNotFoundError";
                break;
                default:
                    qDebug() << "Error";
            }
        } else {
            QPixmap pixmap;
            pixmap.loadFromData(reply->readAll());
            emit loaded(pixmap);
            reply->deleteLater();
        }
    }
}

QByteArray QrzruCallbook::Request(QString url) {
    QNetworkAccessManager manager;
    QEventLoop eventLoop;
    QTimer tot;
    QByteArray data;

    QObject::connect(&tot, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(&manager, &QNetworkAccessManager::finished,
                     &eventLoop, &QEventLoop::quit);

    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "MiniQSOLogger");

    QNetworkReply* reply = manager.get(request);
    tot.start(1000);
    eventLoop.exec();

    if (tot.isActive()) {
        tot.stop();
        if (reply->error() != QNetworkReply::NoError) {
            switch (reply->error()) {
                case QNetworkReply::ContentNotFoundError:
                    emit error404();
                break;
                default:
                    emit error();
            }
        } else {
            data = reply->readAll();
            reply->deleteLater();
        }
    }

    return data;
}

QString QrzruCallbook::getTagValue(QByteArray data, QString tag) {
    QString value = "";
    QXmlStreamReader xml;

    xml.addData(data);
    while(!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) continue;
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == tag) {
                xml.readNext();
                value = xml.text().toString();
            }
        }
    }

    xml.clear();
    return value;
}
