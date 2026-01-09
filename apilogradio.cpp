#include "apilogradio.h"

APILogRadio::APILogRadio(QString APILogRadioAccessToken, QObject *parent)
    : QObject{parent}
{
    this->APILogRadioAccessToken = APILogRadioAccessToken;
}
//--------------------------------------------------------------------------------------------------------------------

void APILogRadio::getToken()
{
    QNetworkRequest request(QUrl("https://api.logradio.ru/user/api-token"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader(QByteArrayLiteral("X-Application-Key"), QString(XApplicationKey).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QJsonObject body;
    body["vendor"] = "QSO.SU";
    body["name"] = "QSOLogger";
    body["version"] = "1.0";

    QJsonDocument doc(body);
    QByteArray jsonBA = doc.toJson();
    qDebug().noquote() << "Get token from logradio.ru. " << jsonBA;

    QNetworkReply *reply = m_manager.post(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug() << "API LogRadio status сode: " << status_code.toInt();

        switch(status_code.toInt()) {
        case 200: {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            QJsonValue access_token = jsonDocument["access_token"].toString();
            QJsonValue confirmation_key = jsonDocument["confirmation_key"].toString();
            QJsonValue confirmation_after = jsonDocument["confirmation_after"].toString();
            QJsonValue confirmation_before = jsonDocument["confirmation_before"].toString();
            QJsonValue valid_after = jsonDocument["valid_after"].toString();
            QJsonValue valid_before = jsonDocument["valid_before"].toString();
            emit received(access_token.toString(), confirmation_key.toString(), confirmation_after.toString(), confirmation_before.toString(), valid_after.toString(), valid_before.toString());
            break;
        };
        default: {
            qDebug() << "Error get token from LogRadio.ru. " << status_code.toInt();
            emit errorGetToken();
            break;
        }
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

bool APILogRadio::checkToken()
{
    QNetworkRequest request(QUrl("https://api.logradio.ru/user/api-token"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + APILogRadioAccessToken).toUtf8());
    request.setRawHeader(QByteArrayLiteral("X-Application-Key"), QString(XApplicationKey).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug() << "Check LogRadio.ru token...";

        switch(status_code.toInt()) {
             case 204:
                qDebug() << "The token is correct. " << status_code.toInt();
                emit checked(status_code.toInt(), tr("Токен активен."));
                break;
             case 401:
                qDebug() << "The token is not linked to the user. " << status_code.toInt();
                emit checked(status_code.toInt(), tr("Токен не привязан к пользователю."));
                break;
             default:
                qDebug() << "The token is incorrect." << status_code.toInt();
                emit checked(status_code.toInt(), tr("Токен не корректный."));
         }
        reply->deleteLater();
        return true;
    });
    return false;
}

//--------------------------------------------------------------------------------------------------------------------

void APILogRadio::SendQso(QVariantList data) {
    QNetworkRequest request(QUrl("https://api.logradio.ru/ham/qso"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json, text/javascript, */*; q=0.01");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + APILogRadioAccessToken).toUtf8());
    request.setRawHeader(QByteArrayLiteral("X-Application-Key"), QString(XApplicationKey).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    // --- Формирование JSON ---
    int dbid = data.value(0).toInt();

    QJsonObject QSO_Obj;
    QSO_Obj["id"] = dbid;
    QSO_Obj["comm_datetime_at"] = data.value(7).toString() + "+00";
    QSO_Obj["callsign_s"] = data.value(16).toString();
    QSO_Obj["callsign_r"] = data.value(3).toString();
    QSO_Obj["operator_s"] = data.value(17).toString();
    QSO_Obj["frequency"] = data.value(6).toLongLong();
    QSO_Obj["mode"] = data.value(5).toString();
    QSO_Obj["submode"] = "";

    QString mode = data.value(5).toString();
    if (mode == "LSB") { QSO_Obj["mode"] = "SSB"; QSO_Obj["submode"] = "LSB"; }
    else if (mode == "USB") { QSO_Obj["mode"] = "SSB"; QSO_Obj["submode"] = "USB"; }
    else if (mode == "CW")  { QSO_Obj["mode"] = "CW";  QSO_Obj["submode"] = "PCW"; }
    else if (mode == "FT4") { QSO_Obj["mode"] = "MFSK"; QSO_Obj["submode"] = "FT4"; }
    else if (mode == "FT8") { QSO_Obj["mode"] = "FT8"; QSO_Obj["submode"] = ""; }

    QSO_Obj["band"]     = data.value(4).toString();
    QSO_Obj["rst_s"]    = data.value(9).toString();
    QSO_Obj["rst_r"]    = data.value(10).toString();
    QSO_Obj["name_r"]   = data.value(8).toString();
    QSO_Obj["cnty_s"]   = data.value(14).toString();
    QSO_Obj["rda_s"]    = data.value(15).toString();
    QSO_Obj["cnty_r"]   = data.value(13).toString();
    QSO_Obj["region_r"] = data.value(13).toString();
    QSO_Obj["qth_r"]    = data.value(11).toString();
    QSO_Obj["qthloc_r"] = data.value(13).toString();
    QSO_Obj["rda_r"]    = data.value(12).toString();
    QSO_Obj["comment"]  = "QSO.SU";

    QJsonObject QSO_Data;
    QSO_Data["number"] = dbid;
    QSO_Data["data"] = QSO_Obj;

    qDebug() << data;

    QJsonArray QSO_Array;
    QSO_Array.append(QSO_Data);
    QJsonDocument doc(QSO_Array);
    QByteArray jsonBA = doc.toJson();
    qDebug().noquote() << "Sending QSO data to LogRadio.ru." << jsonBA;

    // --- Отправка запроса ---
    QNetworkReply *reply = m_manager.post(request, jsonBA);

    // --- Таймаут (5 секунд) ---
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    timer->start(5000); // 5 секунд
    connect(timer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning()) {
            qWarning() << "LogRadio.ru request timeout!";
            reply->abort();
        }
    });

    // --- Обработка ответа ---
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "LogRadio.ru request failed:" << reply->errorString();
            emit QSOStatus(true); // ошибка
            reply->deleteLater();
            return;
        }

        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QByteArray data = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

        if (!jsonDocument.isArray()) {
            qWarning() << "LogRadio.ru Invalid JSON response:" << data;
            emit QSOStatus(true);
            reply->deleteLater();
            return;
        }

        QJsonArray jsonArray = jsonDocument.array();
        if (jsonArray.isEmpty()) {
            qWarning() << "LogRadio.ru Empty JSON response";
            emit QSOStatus(true);
            reply->deleteLater();
            return;
        }

        QJsonObject jo = jsonArray.at(0).toObject();
        bool error = jo.value("error").toBool();
        int number = jo.value("number").toInt();

        switch (status_code.toInt()) {
        case 200:
            qDebug() << "LogRadio.ru Network reply finished. Code:" << status_code.toInt();
            if (!error) {
                emit synced(dbid);
                qDebug() << "LogRadio.ru QSO №" << number << " sent successfully.";
            } else {
                qWarning() << "LogRadio.ru Error! QSO №" << number << " not sent.";
                qWarning() << "LogRadio.ru " << jsonDocument;
            }
            emit QSOStatus(error);
            break;
        default:
            qWarning() << "LogRadio.ru Network reply finished. Code:" << status_code.toInt();
            emit QSOStatus(true);
            break;
        }

        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------
