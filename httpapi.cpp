/**********************************************************************************************************
Description :  Implementation of the HttpApi class, which provides interaction with the QSO.SU API service.
            :  Supports authentication, sending and updating QSOs, managing callsigns, retrieving
            :  reference data (bands, submodes, hamdefs), geolocation queries, callbook lookups,
            :  logs synchronization, magnetic storm info, spot servers, and integrated chat features.
Version     :  3.5.2
Date        :  10.09.2025
Author      :  R9JAU
Comments    :  - Uses QNetworkAccessManager with SSL for HTTP/HTTPS requests.
            :  - Implements keepalive mechanism with configurable timeout (KEEPALIVE_INTERVAL).
            :  - Supports proxy configuration (SOCKS5, HTTP).
***********************************************************************************************************/

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

void HttpApi::setAccessToken(QString token)
{
    this->accessToken = token;
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::configureProxy(int type, QString proxyHost, int proxyPort, QString user, QString password)
{
    QNetworkProxy::ProxyType pType;

    switch(type) {
        case 0: pType = QNetworkProxy::HttpProxy;
        break;
        case 1: pType = QNetworkProxy::Socks5Proxy;
        break;
        case 2: pType = QNetworkProxy::Socks5Proxy;
        break;
    }
    QNetworkProxy proxy(pType, proxyHost, proxyPort);

    proxy.setUser(user);
    proxy.setPassword(password);
    m_manager.setProxy(proxy);
}

//--------------------------------------------------------------------------------------------------------------------

void HttpApi::SendQso(QVariantList data) {
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/sendLog")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
    body["my_gridsquare"] = data.value(14).toString(); //Add bugFix
    body["my_cnty"] = data.value(15).toString();       //Add bugFix

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
            default: {
                emit syncerror(dbid, hash.toString());
                qDebug() << "Responce: " << data;
            }
        }
       reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getCallsign()
{
  if (accessToken.length() == 0) {
      qDebug() << "getCallsign";
      emit emptyToken();
      return;
  }
  QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getCallsign")));
  request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
        qDebug() << "addCallsign";
        emit emptyToken();
        return;
    }

    QNetworkRequest request(QUrl("https://api.qso.su/method/v1/addCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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

    connect(reply, &QNetworkReply::finished, this, [=]()
    {
        QByteArray repdata = reply->readAll();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(repdata);
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        QJsonObject rootObj = jsonDocument.object();

        if (jsonDocument.object().contains("error") && rootObj["error"].isObject()) {
            if (rootObj.contains("error") && rootObj["error"].isObject()) {
                QJsonObject errorObj = rootObj["error"].toObject();

                // Получаем список ключей
                QStringList keys = errorObj.keys();
                qDebug() << "ERROR Code:" << status_code.toInt() << errorObj;
            }
        }

        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Callsign added. Code:" << status_code.toInt();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::checkStatusCallsign(QString callsign)
{
    if (accessToken.length() == 0) {
        qDebug() << "checkStatusCallsign";
        emit emptyToken();
        return;
    }

    QJsonObject body;
    body["callsign"] = callsign;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/checkStatusCallsign"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
        qDebug() << "getListSubmodeDropDown";
        emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListSubmodeDropDown")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
        qDebug() << "getListBand";
        emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getListBand")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
            qDebug() << "Файл HamDefs.xml успешно загружен, размер:" << data.size() << "байт";
            XMLdata.append(data);
            emit HamDefsUploaded();
        } else {
            //emit error(reply->error());
            qDebug() << "Ошибка загрузки файла HamDefs.xml.";
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
        qDebug() << "deleteByHashLog";
        emit emptyToken();
        return;
    }

    QNetworkRequest request(QUrl("https://api.qso.su/method/v1/deleteByHashLog"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
        qDebug() << "getGeocodeByLocator";
        emit emptyToken();
        return;
    }

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getGeocodeByLocator?locator="+Locator));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
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

            //qDebug() << "Geolocation Data: " << data;
            if (jsonDocument.object().contains("error")) {
                QJsonObject errorObject = jsonDocument["error"].toObject();
                qDebug() << "ERROR:" << errorObject["name"].toString() << errorObject["message"].toString();
                emit LocRecceived();
                return;
            }
            QJsonObject response = jsonDocument["response"].toObject();
            QJsonObject center = response["center"].toObject();
            userData.lat = center["lat"].toDouble();
            userData.lng = center["lng"].toDouble();
            emit LocRecceived();
        } else {
            qDebug() << "getGeocodeByLocator error: " << reply->errorString();
            emit LocRecceived();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getConfirmedLogs(QString date, int callsign_id)
{
    if (accessToken.length() == 0) {
        qDebug() << "getConfirmedLogs";
        emit emptyToken();
        return;
    }

    QJsonObject body;
    body["date"] = date;
    body["id_station_callsign"] = callsign_id;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getConfirmedLogs"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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

            foreach (const QJsonValue &c, cnfrQSOArray) {
                cnfrQSOs.append(c.toObject().toVariantMap());
            }
            emit confirmQSOs(count.toInt());
        } else {
            qDebug() << "Error getConfirmedLogs: " << reply->errorString();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getUser()
{
    if (accessToken.length() == 0) {
        qDebug() << "getUser";
        emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/getUser")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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
           qDebug() << "Error getUser: " << reply->errorString();
           emit error(reply->error());
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getCallbook(QString callsign)
{
    if (accessToken.length() == 0) {
        qDebug() << "getCallbook";
        emit emptyToken();
        return;
    }

    QJsonObject body;
    body["callsign"] = callsign;
    QJsonDocument doc(body);

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v1/getCallBook"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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

        if (reply->error() != QNetworkReply::NoError) {
            QVariant error_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            QByteArray dt = reply->readAll();
            QJsonDocument json = QJsonDocument::fromJson(dt);
            qDebug().noquote() << "Error in getCallbook: Code " << error_code.toInt() << " JSON_BODY " << json;
        }

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

            //qDebug().noquote() << jsonDocument;
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
            QJsonObject geoObj = response.value("gridsquare_geo").toObject();
            QJsonObject centerObj = geoObj.value("center").toObject();
            double centerLat = centerObj.value("latitude").toDouble();
            double centerLon = centerObj.value("longitude").toDouble();

            QString photo = "";
            if(files.count() > 0) photo = files.first().toString();
            else photo = "";
            callsignInfo.clear();
            callsignInfo << name << qth << gridsquare << cnty << QString::number(user) << QString::number(srr) << prefix << prefix_dxcc << countryName << countryCode;
            callsignInfo << QString::number(ituz) << QString::number(cqz) << photo << QString::number(centerLat) << QString::number(centerLon);
            emit userDataUpdated(!callsignInfo.isEmpty() && !callsignInfo.at(0).isEmpty());
        } else {
            qDebug() << "Error getCallbook: " << reply->errorString();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getPing()
{
    QNetworkRequest request((QUrl("https://qso.su")));

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            serviceAvailable = true;
            emit serviceAvailableChanged(true);
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
        qDebug() << "updateByHashLog";
        emit emptyToken();
        return;
    }

    QNetworkRequest request((QUrl("https://api.qso.su/method/v1/updateByHashLog")));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
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

        if (reply->error() != QNetworkReply::NoError) {
            QVariant error_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            QByteArray dt = reply->readAll();
            QJsonDocument json = QJsonDocument::fromJson(dt);
            qDebug().noquote() << "Error: Code " << error_code.toInt() << " JSON_BODY " << json;
        }

        if (reply->error() == QNetworkReply::NoError) {
            QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

            qDebug().noquote() << jsonDocument;

            switch(status_code.toInt())
            {
            case 200: {
                QString new_hash = jsonDocument["response"].toString();
                emit QSODataUpdated(new_hash);
                qDebug() << "QSO updated. Code:" << status_code.toInt() << " New HASH code:" << new_hash;
                break;
            }
            default: {
                if (jsonDocument.object().contains("error")) {
                    QJsonObject errorObject = jsonDocument["error"].toObject();
                    qDebug() << "ERROR:" << errorObject["hash"].toArray() << errorObject["message"].toString();
                    emit errorQSODataUpdated(errorObject["message"].toString());
                }
                break;
            }
            }
            reply->deleteLater();
        } else qDebug() << "Error updateByHashLog: " << reply->errorString();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getLogs(int operator_id, int station_id, int page, int count)
{    
    if (accessToken.length() == 0) {
        qDebug() << "getLogs";
        emit emptyToken();
        return;
    }

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v2/getLogs?operator_id="+QString::number(operator_id)+"&station_id="+QString::number(station_id)+"&page="+QString::number(page)+"&count="+QString::number(count)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            static int cnt;
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

            QJsonObject rootObj = jsonDocument.object();
            QJsonObject responseObj = rootObj.value("response").toObject();

            int totalCount = responseObj.value("total_count").toInt();
            int countLogs = responseObj.value("count_logs").toInt();

            uploadLogs.clear();
            QJsonArray logsArray = responseObj.value("logs").toArray();
            for (const QJsonValue& logValue : logsArray)
            {
                cnt++;
                uploadLogs.append(logValue.toObject().toVariantMap());
            }
            emit uploadQSOs(totalCount);
        } else {
            qDebug() << "Error Upload Logs From QSO.SU: " << reply->errorString();
            QByteArray dt = reply->readAll();
            qDebug() << "Error body: " << dt;
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getMagneticStormHistory()
{
    if (accessToken.length() == 0) {
        //qDebug() << "getMagneticStormHistory";
        //emit emptyToken();
        return;
    }

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v2/getMagneticStormHistory"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
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
            QJsonObject rootObj = jsonDocument.object();
            QJsonObject responseObj = rootObj.value("response").toObject();
            QJsonArray indicesArray = responseObj.value("indices").toArray();
            emit MagStormUpdated(indicesArray);
        } else {
            emit error(reply->error());
            qDebug() << "Error Upload Magnetic Storm Info: " << reply->errorString() << "data: " << reply->readAll();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getMagneticStormCurrent()
{
    if (accessToken.length() == 0) {
        //qDebug() << "getMagneticStormCurrent";
        //emit emptyToken();
        return;
    }

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v2/getMagneticStormCurrent"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
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
            QJsonObject rootObj = jsonDocument.object();
            QJsonObject responseObj = rootObj.value("response").toObject();
            emit MagStormCurrentUpdated(responseObj);
        } else {
            emit error(reply->error());
            qDebug() << "Error Upload Magnetic Storm Info: " << reply->errorString() << "data: " << reply->readAll();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getListSpotServers()
{
    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v2/getServers"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
            QJsonObject root = jsonDocument.object();
            QJsonObject response = root.value("response").toObject();
            QJsonArray servers = response.value("servers").toArray();

            QList<ServerInfo> serverList;

            for (const QJsonValue &serverVal : servers) {
                QJsonObject server = serverVal.toObject();
                ServerInfo info;
                info.host = server.value("host").toString();
                info.port = server.value("port").toInt();
                info.client = server.value("client").toInt();
                serverList.append(info);
            }
            emit spotServersReceived(serverList);
        } else {
            emit error(reply->error());
            qDebug() << "Error Upload ListSpotServers: " << reply->errorString() << "data: " << reply->readAll();
        }
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getChats()
{
    if (accessToken.length() == 0) {
        //qDebug() << "getChats";
        //emit emptyToken();
        return;
    }

    QNetworkRequest request = QNetworkRequest(QUrl("https://api.qso.su/method/v2/getChats"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QList<Chat> chats;
        chats.clear();
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isObject()) {
            QJsonArray chatArray = doc["response"].toObject()["chats"].toArray();
            for (const QJsonValue &val : chatArray) {
                QJsonObject c = val.toObject();
                Chat chat;
                chat.id = c["id"].toInt();
                chat.name = c["name"].toString();
                chat.isTemporary = c["is_temporary"].toBool();
                chat.expiresAt = c["expires_at"].toString();
                chats.append(chat);
            }
        }
        emit chatsLoaded(chats);
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::getChatMessages(int chatId)
{
    if (accessToken.isEmpty()) {
        qDebug() << "getChatHistory: empty token";
        emit emptyToken();
        return;
    }

    QNetworkRequest request(QUrl("https://api.qso.su/method/v2/getChatHistory?id=" + QString::number(chatId)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-operating-system", XOperatingSystem.toUtf8());
    request.setRawHeader("x-device-name", XDeviceName.toUtf8());
    request.setRawHeader("x-version-logger", XVersionLogger.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QNetworkReply *reply = m_manager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            //reply->deleteLater();
            //return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();

        // Проверка на наличие ошибки сервера
        if (obj.contains("error")) {
            QJsonObject err = obj["error"].toObject();
            QString msg = QString::fromUtf8(QJsonDocument(err).toJson(QJsonDocument::Compact));
            qDebug() << "Server error:" << msg;
            reply->deleteLater();
            return;
        }

        Chat chat;
        QList<Message> messages;

        if (obj.contains("response")) {
            QJsonObject resp = obj["response"].toObject();

            // Чат
            QJsonObject chatObj = resp["chat"].toObject();
            chat.id = chatObj["id"].toInt();
            chat.name = chatObj["name"].toString();
            chat.isTemporary = chatObj["is_temporary"].toBool();
            chat.expiresAt = chatObj["expires_at"].toString();

            // Сообщения
            QJsonArray msgArray = resp["messages"].toArray();
            for (const QJsonValue &val : msgArray) {
                QJsonObject m = val.toObject();
                Message msg;
                msg.id = m["id"].toInt();
                msg.chatId = m["id_chat"].toInt();
                msg.sender = m["sender"].toString();
                msg.text = m["text"].toString();
                msg.textColor = m["text_color"].toString();
                msg.backgroundColor = m["background_color"].toString();
                msg.sentAt = m["sent_at"].toString();
                messages.append(msg);
            }
        }
        emit chatWithMessagesLoaded(chat, messages);
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::sendMessage(int id_chat, int id_callsign, QString message)
{
    if (accessToken.isEmpty()) {
        qDebug() << "sendMessage";
        emit emptyToken();
        return;
    }

    QUrl url("https://api.qso.su/method/v2/sendMessage");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    // Формируем тело запроса
    QUrlQuery params;
    params.addQueryItem("id_chat", QString::number(id_chat));
    params.addQueryItem("id_callsign", QString::number(id_callsign));
    params.addQueryItem("message", message);

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *reply = m_manager.post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [this, reply, id_chat]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Ошибка отправки: " + reply->errorString());
            qDebug() << "Ошибка отправки: " + reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit errorOccurred("Ошибка парсинга ответа на отправку: " + parseError.errorString());
            qDebug() << "Ошибка парсинга ответа на отправку: " + parseError.errorString();
            reply->deleteLater();
            return;
        }

        if (!doc.isObject()) {
            emit errorOccurred("Некорректный ответ от API при отправке");
            qDebug() << "Некорректный ответ от API при отправке";
            reply->deleteLater();
            return;
        }

        QJsonObject msgObj = doc["response"].toObject()["message"].toObject();
        Message msg;
        msg.id = msgObj["id"].toInt();
        msg.chatId = msgObj["id_chat"].toInt();
        msg.sender = msgObj["sender"].toString();
        msg.text = msgObj["text"].toString();
        msg.textColor = msgObj["text_color"].toString();
        msg.backgroundColor = msgObj["background_color"].toString();
        msg.sentAt = msgObj["sent_at"].toString();

        emit messageSent(id_chat, msg);
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

void HttpApi::sendSpot(QString hash, QString comment) {
    if (accessToken.length() == 0) {
        emit emptyToken();
        return;
    }

    QUrl url("https://api.qso.su/method/v2/pushSpotMessage");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QSO.SU Agent v3.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader(QByteArrayLiteral("x-operating-system"), QString(XOperatingSystem).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-device-name"), QString(XDeviceName).toUtf8());
    request.setRawHeader(QByteArrayLiteral("x-version-logger"), QString(XVersionLogger).toUtf8());
    request.setRawHeader(QByteArrayLiteral("Authorization"), QString("Bearer " + accessToken).toUtf8());
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

    QUrlQuery params;
    params.addQueryItem("hash", hash);
    params.addQueryItem("comment", comment);

    QByteArray postData = params.toString(QUrl::FullyEncoded).toUtf8();
    QNetworkReply *reply = m_manager.post(request, postData);
    qDebug().noquote() << "POST data:" << QString::fromUtf8(postData);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Ошибка отправки спота: " + reply->errorString());
            qDebug() << "Ошибка отправки спота: " + reply->errorString();

            QByteArray raw = reply->readAll();

            // Пытаемся распарсить JSON
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(raw, &err);

            if (err.error == QJsonParseError::NoError) {
                // Вывод JSON в читаемом виде
                qDebug().noquote() << QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
            } else {
                // Если JSON битый — просто выводим строку как UTF-8
                qDebug().noquote() << QString::fromUtf8(raw);
            }
            reply->deleteLater();
            return;
        }
        qDebug().noquote() << reply->readAll();
        reply->deleteLater();
    });
}
//--------------------------------------------------------------------------------------------------------------------

