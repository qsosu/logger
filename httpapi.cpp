#include "httpapi.h"

#define DEBUG

HttpApi::HttpApi(QSqlDatabase db, QString accessToken, QObject *parent)
  : QObject{parent}
{
  this->db = db;
  this->accessToken = accessToken;
}

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

/* TEST */
void HttpApi::getCallsign() {
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
