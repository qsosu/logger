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
    request.setRawHeader(QByteArrayLiteral("X-Application-Key"), QString("f23005c5-681e-43fb-96fa-6e80c89cbb9c").toUtf8());
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
        qDebug() << "API LogRadio Ansver сode: " << status_code.toInt();

        QByteArray data = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
        QJsonValue access_token = jsonDocument["access_token"].toString();
        QJsonValue confirmation_key = jsonDocument["confirmation_key"].toString();
        QJsonValue confirmation_after = jsonDocument["confirmation_after"].toString();
        QJsonValue confirmation_before = jsonDocument["confirmation_before"].toString();
        QJsonValue valid_after = jsonDocument["valid_after"].toString();
        QJsonValue valid_before = jsonDocument["valid_before"].toString();

        emit received(access_token.toString(), confirmation_key.toString(), confirmation_after.toString(), confirmation_before.toString(), valid_after.toString(), valid_before.toString());
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
                emit checked(status_code.toInt(), "The token is correct.");
                break;
             case 401:
                qDebug() << "The token is not linked to the user. " << status_code.toInt();
                emit checked(status_code.toInt(), "The token is not linked to the user.");
                break;
             default:
                qDebug() << "The token is incorrect." << status_code.toInt();
                emit checked(status_code.toInt(), "The token is incorrect.");
         }
        QByteArray data = reply->readAll();
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

    QJsonObject QSO_Obj;
    int dbid = data.value(0).toInt();
    QSO_Obj["id"] = data.value(0).toInt();
    QSO_Obj["comm_datetime_at"] = data.value(7).toString();
    QSO_Obj["callsign_s"] = data.value(16).toString();
    QSO_Obj["callsign_r"] = data.value(3).toString();
    QSO_Obj["frequency"] = data.value(6).toLongLong();

    QSO_Obj["mode"] = data.value(5).toString();
    QSO_Obj["submode"] = "";

    if(data.value(5).toString()=="LSB") {
        QSO_Obj["mode"] = "SSB";
        QSO_Obj["submode"] = "LSB";
    } else
    if(data.value(5).toString()=="USB") {
        QSO_Obj["mode"] = "SSB";
        QSO_Obj["submode"] = "USB";
    } else
    if(data.value(5).toString()=="CW") {
        QSO_Obj["mode"] = "CW";
        QSO_Obj["submode"] = "PCW";
    } else
    if(data.value(5).toString()=="FT4") {
        QSO_Obj["mode"] = "MFSK";
        QSO_Obj["submode"] = "FT4";
    } else
    if(data.value(5).toString()=="FT8") {
        QSO_Obj["mode"] = "FT8";
        QSO_Obj["submode"] = "";
    }

    QSO_Obj["band"] = data.value(4).toString();
    QSO_Obj["rst_s"] = data.value(9).toString();
    QSO_Obj["rst_r"] = data.value(10).toString();
    QSO_Obj["name_r"] = data.value(8).toString();
    QSO_Obj["cnty_s"] = data.value(14).toString();
    QSO_Obj["region_s"] = data.value(15).toString();
    //QSO_Obj["rda_s"] = data.value(14).toString();
    QSO_Obj["cnty_r"] = data.value(12).toString();
    QSO_Obj["region_r"] =  data.value(13).toString();
    QSO_Obj["qth_r"] = data.value(11).toString();
    QSO_Obj["qthloc_r"] = data.value(17).toString();
    QSO_Obj["rda_r"] = data.value(12).toString();
    QSO_Obj["comment"] = "QSO.SU";

    QJsonObject QSO_Data;
    QSO_Data["number"] = data.value(0).toInt();
    QSO_Data["data"] = QSO_Obj;

    QJsonArray QSO_Array;
    QSO_Array.append(QSO_Data);

    QJsonDocument doc(QSO_Array);
    QByteArray jsonBA = doc.toJson();

    qDebug().noquote() << "Sending QSO data to LogRadio.ru." << jsonBA;

    QNetworkReply *reply = m_manager.post(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QByteArray data = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
        QJsonArray jsonArray = jsonDocument.array();
        QJsonObject jo = jsonArray.at(0).toObject();

        bool error = jo.value("error").toBool();
        int number = jo.value("number").toInt();

        switch(status_code.toInt()) {
            case 200: {
                qDebug() << "LogRadio.ru Network reply finished. Code:" << status_code.toInt();
                if(error == 0) {
                    emit synced(dbid);
                    qDebug() << "QSO №" << number << " sended to LogRadio.ru service.";
                }
                else {
                    qDebug() << "LogRadio.ru Error! QSO №" << number << " not sended to LogRadio.ru service.";
                    qDebug() << jsonDocument;
                }
                emit QSOStatus(error);
                break;
            };
            default:
                qDebug() << "LogRadio.ru Network reply finished. Code:" << status_code.toInt();
                break;
        }
       reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------
