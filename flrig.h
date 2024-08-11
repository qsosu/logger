#ifndef FLRIG_H
#define FLRIG_H

#include <QObject>
#include <QTimer>
#include "maia/maiaXmlRpcClient.h"

class Flrig : public QObject
{
  Q_OBJECT
public:
  explicit Flrig(QHostAddress host, uint16_t port, uint16_t updateInterval, QObject *parent = nullptr);
  void start();
  void stop();
  QString getVersion();
  unsigned int getFrequencyHz();
  QString getMode();
  QString getErrorString();
  int getErrorCode();
  bool getConnState();

private:
  MaiaXmlRpcClient *rpc;

  QHostAddress host;
  uint16_t port;
  uint16_t updateInterval;
  QTimer *updateTimer;
  int errorCode;
  QString errorString;
  QString flrigVersion;
  unsigned int frequencyHz;
  QString mode;
  bool rpc_connected;

private slots:
  void requestRpcData();
  void rpcResponseInfo(QVariant &);
  void rpcResponseFreq(QVariant &);
  void rpcResponseMode(QVariant &);
  void rpcDefaultResponse(QVariant &arg);
  void rpcFault(int, const QString &);

signals:
  void connected();
  void disconnected();
  void rpcError();
  void updated();

};

#endif // FLRIG_H
