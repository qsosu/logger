#ifndef HTTPAPI_H
#define HTTPAPI_H

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

QT_REQUIRE_CONFIG(ssl);

#define KEEPALIVE_INTERVAL 5000
#define HTTP_TIMEOUT 3000

class HttpApi : public QObject
{
  Q_OBJECT
public:
  explicit HttpApi(QSqlDatabase db, QString accessToken, QObject *parent = nullptr);
  void SendQso(QVariantList data);
  void getUser();
  void getCallsign();
  void addCallsign(QVariantList data);
  void checkStatusCallsign(QString callsign);
  void getListSubmodeDropDown();
  void getListBand();
  void getGeocodeByLocator(QString Locator);
  void getConfirmedLogs();
  void loadHamDefs();
  void deleteByHashLog(QString hash); //Удаление QSO из радиолюбительского журнала
  void getCallbook(QString callsign);
  void getPing();
  void updateByHashLog(QVariantList data);

  bool serviceAvailable;
  QVector<QVariantMap> callsigns;
  QStringList modulations;
  QStringList bands;
  QStringList callsignInfo;
  QByteArray XMLdata;
  QVector<QVariantMap> cnfrQSOs;

  typedef struct userData {
      int id;
      QString lang;
      QString last_activity;
      bool premium;
      QString premium_time;
      QString registered;
  } userData_t;
  userData_t userData;

private:
  enum Method {
    GET,
    POST,
    DELETE
  };

  QByteArray request(Method method, QString token, QString url, QByteArray postData = nullptr);
  void errorHandle(QNetworkReply::NetworkError error);
  void requestKeepalive();

  QString accessToken;
  QString XOperatingSystem;
  QString XDeviceName;
  QString XVersionLogger;
  QSqlDatabase db;
  QTimer *keepaliveTimer;
  unsigned int serverTimestamp = 0;
  QNetworkAccessManager m_manager;
  QStringList userDataList;


signals:
  void emptyToken();
  void available();
  void unavailable();
  void confirmQSOs();
  void accountDataUpdated();
  void callsignsUpdated();
  void callsignStatus(int);
  void synced(int, QString);
  void syncerror(int, QString);
  void error(QNetworkReply::NetworkError);
  void modesUpdated();
  void bandsUpdated();
  void HamDefsUploaded();
  void HamDefsError();
  void userDataUpdated();
  void getUserInfo(QStringList);
  void QSODataUpdated(QString);
  void errorQSODataUpdated(QString);
};

#endif // HTTPAPI_H
