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
  void getCallsign();

  bool serviceAvailable;
  QVector<QVariantMap> callsigns;
  //QVector<QVariantMap> operators;

private:
  enum Method {
    GET,
    POST
  };

  QByteArray request(Method method, QString token, QString url, QByteArray postData = nullptr);
  void errorHandle(QNetworkReply::NetworkError error);
  void requestKeepalive();

  QString accessToken;
  QSqlDatabase db;
  QTimer *keepaliveTimer;

  unsigned int serverTimestamp = 0;

  QNetworkAccessManager m_manager;
  QNetworkReply *m_reply = nullptr;

private slots:


signals:
  void emptyToken();
  void available();
  void unavailable();
  void accountDataUpdated();

  void callsignsUpdated();
  void synced(int);
  void syncerror(int);
  void error(QNetworkReply::NetworkError);

};

#endif // HTTPAPI_H
