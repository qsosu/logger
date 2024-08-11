#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

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

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(QObject *parent = nullptr);
    bool start(uint16_t port);

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
    uint16_t port;

private slots:
    void onReadyRead();
    void process(QByteArray data);

signals:
    void heartbeat();
    void logged();
};

#endif // UDPRECEIVER_H
