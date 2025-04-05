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
    long freq;
    long old_freq;
    int band;
    int old_band;
    int mode;
    int old_mode;

    void openSerial(QString port);
    void sendCommand(const char *cmd);
    void setBand(int band);
    void setMode(int mode);
    void setFreq(long freq);
    int freqToBand(long freq);
    void convertMode(int mode);

private slots:
    void portRead();
    void handleError(QSerialPort::SerialPortError error);

signals:
    void cat_freq(long);
    void cat_band(int);
    void cat_mode(int);

};

#endif // CAT_INTERFACE_H
