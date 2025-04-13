#include "httpapi.h"
#include "mainwindow.h"


#define DEBUG


HttpApi::HttpApi(QSqlDatabase db, QString accessToken, QObject *parent)
  : QObject{parent}
{
  this->db = db;
  this->accessToken = accessToken;
  serviceAvailable = false;

  XOperatingSystem = QSysInfo::prettyProductName();
  XDeviceName = QSysInfo::machineHostName();
  XVersionLogger = VERSION;
}

//--------------------------------------------------------------------------------------------------------------------

void HttpApi::SendQso(QVariantList data) {
    if (accessToken.length() == 0) {
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/sendLog")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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
    qDebug().noquote() << "Sending QSO data to QSO.SU." << jsonBA;

    QNetworkReply *reply = m_manager.post(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        QByteArray data = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
        QJsonObject response = jsonDocument["response"].toObject();
        QJsonValue hash = response["hash"].toString();
        qDebug() << "QSO.SU Network reply finished. Code:" << status_code.toInt() << " Loaded hash: " << hash.toString();

        switch(status_code.toInt()) {
            case 201:
                emit synced(dbid, hash.toString());
                break;
            case 409:
            default:
                emit syncerror(dbid, hash.toString());
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
  QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getCallsign")));
  request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
  request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
  request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
  request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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

      callsigns.clear();

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

    QNetworkRequest request(QUrl("https://api.qso.su/method/v1/addCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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

    QJsonObject body;
    body["callsign"] = callsign;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/checkStatusCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QByteArray jsonBA = doc.toJson();
    QBuffer *buff = new QBuffer;
    buff->setData(jsonBA);
    buff->open(QIODevice::ReadOnly);

    QNetworkReply *reply = m_manager.sendCustomRequest(request, "GET", buff);
    buff->setParent(reply);
    qDebug().noquote() << "Sending QSO data to QSO.SU." << jsonBA;
    connect(reply, &QNetworkReply::finished, this, [=]() {

        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            qDebug().noquote() << data;

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
        //emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListSubmodeDropDown")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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
        //emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListBand")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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

void HttpApi::loadHamDefs()
{
    QNetworkRequest request((QUrl("https://api.qso.su/HamDefs.xml")));
    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Succesful uploaded HamDefs.xml size:" << data.size() << "bytes";
            XMLdata.append(data);
            emit HamDefsUploaded();
        } else {
            //emit error(reply->error());
            qDebug() << "Error upload HamDefs.xml...";
            emit HamDefsError();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------
//Удаление QSO из радиолюбительского журнала

void HttpApi::deleteByHashLog(QString hash)
{
    if(hash == "") return;
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QNetworkRequest request(QUrl("https://api.qso.su/method/v1/deleteByHashLog"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QJsonObject body;
    body["hash"] = hash;
    QJsonDocument doc(body);

    QByteArray removeData = doc.toJson();
    QBuffer *buff = new QBuffer;
    buff->setData(removeData);
    buff->open(QIODevice::ReadOnly);

    QNetworkReply *reply = m_manager.sendCustomRequest(request, "DELETE", buff);
    buff->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    switch(status_code.toInt())
    {
        case 202:
            qDebug() << "QSO deleted. Code:" << status_code.toInt();
            break;
        case 406:
            qDebug() << "Error deleted QSO. Code:" << status_code.toInt();
            break;
    }
    reply->deleteLater();
   });
}
//--------------------------------------------------------------------------------------------------------------------
//Получение координат по локатору

void HttpApi::getGeocodeByLocator(QString Locator)
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QJsonObject body;
    body["locator"] = Locator;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getGeocodeByLocator"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QByteArray postDataByteArray = doc.toJson();
    QBuffer *buff = new QBuffer;
    buff->setData(postDataByteArray);
    buff->open(QIODevice::ReadOnly);

    QNetworkReply *reply = m_manager.sendCustomRequest(request, "GET", buff);
    buff->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray data = reply->readAll();
        qDebug() << data;

        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << data;
        }
        reply->deleteLater();
    });
}

//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getConfirmedLogs()
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QJsonObject body;
    body["date"] = "2025-01-01";
    body["id_station_callsign"] = 102;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getConfirmedLogs"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

            if (jsonDocument.object().contains("error")) {
              QJsonObject errorObject = jsonDocument["error"].toObject();
              qDebug() << "ERROR:" << errorObject["name"].toString() << errorObject["message"].toString();
              return;
            }

            QJsonObject response = jsonDocument["response"].toObject();
            QJsonValue count = response["count"].toInt();
            QJsonArray cnfrQSOArray = response["logs"].toArray();
            cnfrQSOs.clear();
            //qDebug() << "ConfirmedLogsCount: " << count.toInt();

            foreach (const QJsonValue &c, cnfrQSOArray) {
                cnfrQSOs.append(c.toObject().toVariantMap());
                //qDebug() << "Confirmed: " << c.toObject().toVariantMap() << "\n";
            }
            emit confirmQSOs();
        } else {
            emit error(reply->error());
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getUser()
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getUser")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            QJsonObject response = jsonDocument["response"].toObject();

            double id = response["id"].toInt();
            QString lang = response["lang"].toString();
            ulong last_activity = response["last_activity"].toDouble();
            bool premium = response["premium"].toBool();
            ulong premium_time = response["premium_time"].toDouble();
            ulong registered = response["registered"].toDouble();
            userDataList.clear();
            userDataList << QString::number(id) << lang << QString::number(last_activity) << QString::number(premium) << QString::number(premium_time) << QString::number(registered);
            emit getUserInfo(userDataList);
        } else {
            emit error(reply->error());
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getCallbook(QString callsign)
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QJsonObject body;
    body["callsign"] = callsign;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getCallBook"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
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
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            qDebug().noquote() << jsonDocument;
            QJsonObject response = jsonDocument["response"].toObject();
            QString callsign = response["callsign"].toString();
            bool user = response["user"].toBool();
            bool srr = response["srr"].toBool();
            QString prefix = response["prefix"].toString();
            QString prefix_dxcc = response["prefix_dxcc"].toString();
            QString name = response["name"].toString();
            QString gridsquare = response["gridsquare"].toString();
            QString countryName = response["countryName"].toString();
            QString countryCode = response["countryCode"].toString();
            QString cnty = response["cnty"].toString();
            QString qth = response["qth"].toString();
            int ituz = response["ituz"].toInt();
            int cqz = response["cqz"].toInt();
            QJsonArray files = response["files"].toArray();

            QString photo = "";
            if(files.count() > 0) photo = files.first().toString();
            else photo = "";
            callsignInfo.clear();
            callsignInfo << name << qth << gridsquare << cnty << QString::number(user) << QString::number(srr) << prefix << prefix_dxcc << countryName << countryCode << QString::number(ituz) << QString::number(cqz) << photo;
            emit userDataUpdated();
         }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getPing()
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getUser")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            serviceAvailable = true;
        } else {
            serviceAvailable = false;
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------
// Обновление записи QSO по hash. Позволяет обновить данные QSO по уникальному хешу.

void HttpApi::updateByHashLog(QVariantList data)
{
    if (accessToken.length() == 0) {
        //emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/updateByHashLog")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QJsonObject body;
    body["hash"] = data.value(0).toString();
    body["id_operator"] = data.value(1).toInt();
    body["id_station_callsign"] = data.value(2).toInt();
    body["call"] = data.value(3).toString();
    body["name"] = data.value(4).toString();
    body["qth"] = data.value(5).toString();
    body["cnty"] = data.value(6).toString();
    body["gridsquare"] = data.value(7).toString();
    body["rsts"] = data.value(8).toString();
    body["rstr"] = data.value(9).toString();
    body["cqz"] = data.value(10).toInt();
    body["ituz"] = data.value(11).toInt();
    body["datetime"] = data.value(12).toString();
    body["datetime_off"] = data.value(13).toString();
    body["country"] = data.value(14).toString();
    body["cont"] = data.value(15).toString();

    QJsonDocument doc(body);
    QByteArray jsonBA = doc.toJson();
    qDebug().noquote() << "Sending data to service." << jsonBA;

    QNetworkReply *reply = m_manager.put(request, jsonBA);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        QByteArray data = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

        qDebug().noquote() << jsonDocument;

        switch(status_code.toInt())
        {
        case 200: {
            QString new_hash = jsonDocument["response"].toString();
            emit QSODataUpdated(new_hash);
            qDebug() << "QSO updated. Code:" << status_code.toInt();
            qDebug() << "New HASH code:" << new_hash;
            break;
        }
        default: {
            if (jsonDocument.object().contains("error")) {
                QJsonObject errorObject = jsonDocument["error"].toObject();
                qDebug() << "ERROR:" << errorObject["hash"].toArray() << errorObject["message"].toString();
                emit errorQSODataUpdated("Не удалось обновить QSO.");
            }
            break;
        }
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

