#include "flrig.h"

Flrig::Flrig(QHostAddress host, uint16_t port, uint16_t updateInterval, QObject *parent)
  : QObject{parent}
{
  this->host = host;
  this->port = port;
  this->updateInterval = updateInterval;
  rpc_connected = false;
  flrigVersion = "0.0.0";
  frequencyHz = 0;
  mode = "";

  updateTimer = new QTimer(this);
  updateTimer->setInterval(updateInterval);
  connect(updateTimer, &QTimer::timeout, this, &Flrig::requestRpcData);
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::start()
{
  QString rpc_url = QString("http://%1:%2/RPC2").arg(host.toString(), QString::number(port));
  rpc = new MaiaXmlRpcClient(QUrl(rpc_url), this);

  QVariantList args;
  rpc->call("main.get_version", args,
              this, SLOT(rpcResponseInfo(QVariant &)),
              this, SLOT(rpcFault(int, const QString &)));

  requestRpcData();
  updateTimer->start();
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::stop()
{
  updateTimer->stop();
  rpc = Q_NULLPTR;
  rpc_connected = false;
  emit disconnected();
}
//--------------------------------------------------------------------------------------------------------------------

QString Flrig::getVersion()
{
  return flrigVersion;
}
//--------------------------------------------------------------------------------------------------------------------

unsigned int Flrig::getFrequencyHz()
{
  return frequencyHz;
}
//--------------------------------------------------------------------------------------------------------------------

QString Flrig::getMode()
{
  return mode;
}
//--------------------------------------------------------------------------------------------------------------------

int Flrig::getErrorCode()
{
  return errorCode;
}
//--------------------------------------------------------------------------------------------------------------------

QString Flrig::getErrorString()
{
  return errorString;
}
//--------------------------------------------------------------------------------------------------------------------

bool Flrig::getConnState()
{
  return rpc_connected;
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::requestRpcData()
{
  QVariantList args;
  rpc->call("rig.get_vfo", args,
              this, SLOT(rpcResponseFreq(QVariant &)),
              this, SLOT(rpcFault(int, const QString &)));

  rpc->call("rig.get_mode", args,
              this, SLOT(rpcResponseMode(QVariant &)),
              this, SLOT(rpcFault(int, const QString &)));

  emit updated();
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::rpcResponseInfo(QVariant &arg)
{
  if (!rpc_connected) {
    rpc_connected = true;
    emit connected();
  }
  flrigVersion = arg.toString();
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::rpcResponseFreq(QVariant &arg)
{
  if (!rpc_connected) {
    rpc_connected = true;
    emit connected();
  }
  frequencyHz = arg.toInt();
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::rpcResponseMode(QVariant &arg)
{
  if (!rpc_connected) {
    rpc_connected = true;
    emit connected();
  }
  mode = arg.toString();
  if (mode == "USB" || mode == "LSB" || mode == "LSB-D" || mode == "USB-D") mode = "SSB";
  if (mode == "CW-R") mode = "CW";
  if (mode == "FSK-R") mode = "FSK";
  if (mode == "FM-R") mode = "FM";
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::rpcFault(int error, const QString &message)
{
  errorCode = error;
  errorString = message;
  emit rpcError();

  if (rpc_connected) {
    rpc_connected = false;
    emit disconnected();
  }
}
//--------------------------------------------------------------------------------------------------------------------

void Flrig::rpcDefaultResponse(QVariant &arg)
{
  Q_UNUSED(arg);
}
//--------------------------------------------------------------------------------------------------------------------
