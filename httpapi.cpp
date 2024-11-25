#include "httpapi.h"


#define DEBUG


HttpApi::HttpApi(QSqlDatabase db, QString accessToken, QObject *parent)
  : QObject{parent}
{
  this->db = db;
  this->accessToken = accessToken;
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::SendQso(QVariantList data) {
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }
    if (m_reply) {
      m_reply->abort();
      m_reply->deleteLater();
      m_reply = nullptr;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/sendLog")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    int dbid = data.value(0).toInt();

    QJsonObject body;
    body["id_station_callsign"] = data.value(1).toInt();
    body["id_operator"] = data.value(2).toInt();
    body["call"] = data.value(3).toString();
    body["band"] = data.value(4).toString();
    body["mode"] = data.value(5).toString();
    body["freq"] = data.value(6).toLongLong();
    body["datetime"] = data.value(7).toString();
    body["name"] = data.value(8).toString();
    body["rsts"] = data.value(9).toString();
    body["rstr"] = data.value(10).toString();
    body["qth"] = data.value(11).toString();
    body["cnty"] = data.value(12).toString();
    body["gridsquare"] = data.value(13).toString();
    body["my_cnty"] = data.value(14).toString();       //Add bugFix
    body["my_gridsquare"] = data.value(15).toString(); //Add bugFix

    QJsonDocument doc(body);

    QByteArray jsonBA = doc.toJson();
    qDebug().noquote() << "Sending QSO data to service." << jsonBA;

    QNetworkReply *reply = m_manager.post(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug() << "Network reply finished. Code:" << status_code.toInt();
        switch(status_code.toInt()) {
            case 201:
            case 409:
                emit synced(dbid);
            break;
            default:
                emit syncerror(dbid);
        }

       reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getCallsign()
{
  if (accessToken.length() == 0) {
      emit emptyToken();
      return;
  }
  if (m_reply) {
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }
  callsigns.clear();

  QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getCallsign")));
  request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
  request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
  request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

  QNetworkReply *reply = m_manager.get(request);
  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray data = reply->readAll();
      QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

      if (jsonDocument.object().contains("error")) {
        QJsonObject errorObject = jsonDocument["error"].toObject();
        qDebug() << "ERROR:" << errorObject["name"].toString() << errorObject["message"].toString();
        return;
      }

      QJsonObject response = jsonDocument["response"].toObject();
      QJsonArray callsignsArray = response["station_callsign"].toArray();

      foreach (const QJsonValue &c, callsignsArray) {
        callsigns.append(c.toObject().toVariantMap());
      }

      emit callsignsUpdated();
    } else {
      emit error(reply->error());
    }
    reply->deleteLater();
  });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::addCallsign(QVariantList data)
{
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QNetworkRequest request(QUrl("https://api.qso.su/method/v1/addCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QJsonObject body;
    body["callsign"] = data.value(0).toString();
    body["type"] = data.value(1).toInt();
    body["location"] = data.value(2).toString();
    body["rda"] = data.value(3).toString();
    body["ituz"] = data.value(4).toInt();
    body["cqz"] = data.value(5).toInt();

    QDateTime dt;
    body["start_data"] = dt.fromSecsSinceEpoch(data.value(6).toInt()).toString("yyyy-MM-dd hh:mm:ss.zzz");
    body["stop_data"]  = dt.fromSecsSinceEpoch(data.value(7).toInt()).toString("yyyy-MM-dd hh:mm:ss.zzz");

    QJsonDocument doc(body);

    QByteArray jsonBA = doc.toJson();
    qDebug().noquote() << "Sending Callsign for validate to service." << jsonBA;

    QNetworkReply *reply = m_manager.post(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qDebug() << "Network reply finished. Code:" << status_code.toInt();
    reply->deleteLater();
   });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::checkStatusCallsign(QString callsign)
{
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QJsonObject body;
    body["callsign"] = callsign;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/checkStatusCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QByteArray postDataByteArray = doc.toJson();
    QBuffer *buff = new QBuffer;
    buff->setData(postDataByteArray);
    buff->open(QIODevice::ReadOnly);

    QNetworkReply *reply = m_manager.sendCustomRequest(request, "GET", buff);
    buff->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << data;

            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            if (jsonDocument.object().contains("error")) {
                QJsonObject errorObject = jsonDocument["error"].toObject();
                qDebug() << "ERROR:" << errorObject["name"].toString() << errorObject["message"].toString();
                return;
            }
            QJsonObject response = jsonDocument["response"].toObject();
            QJsonValue callsign_status = response["status"].toInt();
            emit callsignStatus(callsign_status.toInt());
        }
        reply->deleteLater();
    });
}

//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getListSubmodeDropDown()
{
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListSubmodeDropDown")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            QJsonObject response = jsonDocument["response"].toObject();

            for(int j = 0; j < response.count(); j++)
            {
               modulations.append(response.value(response.keys().at(j)).toString());
            }
            emit modesUpdated();
        } else {
            emit error(reply->error());
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getListBand()
{
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListBand")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            QJsonObject response = jsonDocument["response"].toObject();

            for(int j = 0; j < response.count(); j++)
            {
                bands.append(response.value(response.keys().at(j)).toString());
            }
            emit bandsUpdated();
        } else {
            emit error(reply->error());
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------















