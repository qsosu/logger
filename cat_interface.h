#ifndef CAT_INTERFACE_H
#define CAT_INTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>


class cat_Interface : public QObject
{
    Q_OBJECT
public:
    explicit cat_Interface(bool catEnable, QObject *parent = nullptr);
    ~cat_Interface();

    bool catEnable;
    QSerialPort *serialPort;
    QTimer *catTimer;
    int catInterval;
    long freq;
    long old_freq;
    int band;
    int old_band;
    int mode;
    int old_mode;
    int res;
    bool VFO;

    bool openSerial(QString port);
    void convertMode(int mode);
    void catSetBaudRate(int baud);
    void catSetParity(QString parity);
    void catSetStopBit(int stopbits);
    void catSetDataBits(int databits);
    void catSetFlowControl(QString flowcontrol);
    void sendCommand(const char *cmd);
    void setBand(int band);
    void setMode(int mode);
    void setFreq(long freq);
    int freqToBand(long freq);
    void setInterval(int interval);

private slots:
    void portRead();
    void handleError(QSerialPort::SerialPortError error);

signals:
    void cat_freq(long);
    void cat_band(int);
    void cat_mode(int);

};

#endif // CAT_INTERFACE_H
