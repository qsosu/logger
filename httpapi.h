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
#include <QList>

QT_REQUIRE_CONFIG(ssl);

#define KEEPALIVE_INTERVAL 5000
#define HTTP_TIMEOUT 3000

struct ServerInfo {
    QString host;
    int port;
    int client;
};

struct Chat {
    int id;
    QString name;
    bool isTemporary;
    QString expiresAt;
};

struct Message {
    int id;
    int chatId;
    QString sender;
    QString text;
    QString textColor;
    QString backgroundColor;
    QString sentAt;
};

class HttpApi : public QObject
{
  Q_OBJECT
public:
  explicit HttpApi(QSqlDatabase db, QString accessToken, QObject *parent = nullptr);
  void SendQso(QVariantList data);
  void setAccessToken(QString token);
  void getUser();
  void getCallsign();
  void addCallsign(QVariantList data);
  void checkStatusCallsign(QString callsign);
  void getListSubmodeDropDown();
  void getListBand();
  void getGeocodeByLocator(QString Locator);
  void getConfirmedLogs(QString date, int callsign_id);
  void loadHamDefs();
  void deleteByHashLog(QString hash); //Удаление QSO из радиолюбительского журнала
  void getCallbook(QString callsign);
  void getPing();
  void updateByHashLog(QVariantList data);
  void configureProxy(int type, QString proxyHost, int proxyPort, QString user, QString password);
  void getLogs(int operator_id, int station_id, int page, int count);
  void getMagneticStormHistory();
  void getMagneticStormCurrent();
  void getListSpotServers();
  void getChats();
  void getChatMessages(int chatId);
  void sendMessage(int id_chat, int id_callsign, QString message);
  void sendSpot(QString hash, QString comment);

  bool serviceAvailable;
  QVector<QVariantMap> callsigns;
  QStringList modulations;
  QStringList bands;
  QStringList callsignInfo;
  QByteArray XMLdata;
  QVector<QVariantMap> cnfrQSOs;
  QVector<QVariantMap> uploadLogs;

  typedef struct userData {
      int id;
      QString lang;
      QString last_activity;
      bool premium;
      QString premium_time;
      QString registered;
      double lat;
      double lng;
  } userData_t;
  userData_t userData;

private:
  enum Method {
    GET,
    POST,
    DELETE
  };

  enum ProxyType {
      Socks5Proxy,
      HttpProxy
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
  void confirmQSOs(int);
  void uploadQSOs(int);
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
  void LocRecceived();
  void MagStormUpdated(QJsonArray);
  void MagStormCurrentUpdated(QJsonObject);
  void spotServersReceived(const QList<ServerInfo> &servers);
  void chatsLoaded(const QList<Chat> &chats);
  void chatWithMessagesLoaded(const Chat &chat, const QList<Message> &messages);
  void messageSent(int chatId, const Message &message);
  void errorOccurred(const QString &errorMessage);
  void serviceAvailableChanged(bool state);
};

#endif // HTTPAPI_H
