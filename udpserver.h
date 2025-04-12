#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QDateTime>

enum MessageType {
    Heartbeat,
    Status,
    Decode,
    Clear,
    Reply,
    QSOLogged,
    Close,
    Replay,
    HaltTx,
    FreeText,
    WSPRDecode,
    Location,
    LoggedADIF,
    HighlightCallsign,
    SwitchConfiguration,
    Configure,
    maximum_message_type
};

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    bool start(uint16_t port);
    bool send(QByteArray data);
    void setRetransl(bool retransl);
    void setRetranslPort(uint16_t port);
    bool retransl;

    /* Message data header */
    quint32 magic;
    quint32 schema;
    quint32 message_type;
    QByteArray id;
    /* Heartbeat data payload*/
    quint32 max_schema_number;
    QByteArray version;
    QByteArray revision;
    /* QSOLogged data payload */
    QDateTime time_off;
    QByteArray dx_call;
    QByteArray dx_grid;
    quint64 tx_frequency_hz;
    QByteArray mode;
    QByteArray report_sent;
    QByteArray report_received;
    QByteArray tx_power;
    QByteArray comments;
    QByteArray name;
    QDateTime time_on;
    QByteArray operator_call;
    QByteArray my_call;
    QByteArray my_grid;
    QByteArray exchange_sent;
    QByteArray exchange_rcvd;
    QByteArray propmode;

private:
    QUdpSocket *socket;
    QUdpSocket *clientSocket;
    uint16_t port;
    uint16_t retransl_port;

private slots:
    void onReadyRead();
    void process(QByteArray data);


signals:
    void heartbeat();
    void logged();
};

#endif // UDPSERVER_H
