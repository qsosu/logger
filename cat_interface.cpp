/**********************************************************************************************************
Description :  cat_Interface class for CAT (Computer Aided Transceiver) control via serial port.
            :  Supports reading and setting frequency, band, and mode of the radio.
            :  Handles serial communication, periodic polling, and converts radio-specific modes to
            :  internal representation.
Version     :  1.0.0
Date        :  10.04.2025
Author      :  R9JAU
Comments    :  - Handles common radios with different FA/FB frequency digit lengths.
**********************************************************************************************************/


#include "cat_interface.h"
#include <QDebug>
#include <string>


#define TIMEOUT 20

cat_Interface::cat_Interface(bool catEnable, QObject *parent)
    : QObject{parent}
{
    this->catEnable = catEnable;
    serialPort = new QSerialPort;
    catTimer = new QTimer(this);

    connect(catTimer, &QTimer::timeout, this, [=]() {
        if(catEnable) {
            sendCommand("FA;");
            sendCommand("MD;");
        }
    });
}
//------------------------------------------------------------------------------------------------------------------------------------------

cat_Interface::~cat_Interface()
{
    if(serialPort->isOpen()) serialPort->close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
            qDebug() << "CAT I/O error on port" << serialPort->portName() << serialPort->errorString();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool cat_Interface::openSerial(QString port)
{    
    serialPort->setPortName(port);
    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "CAT Ошибка, Не удалось подключится к порту. " << serialPort->errorString();
        catTimer->stop();
        return false;
    }
    connect(serialPort, &QSerialPort::readyRead, this, &cat_Interface::portRead);
    connect(serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool cat_Interface::closeSerial()
{
    if(serialPort->isOpen()) serialPort->close();
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::sendCommand(const char *cmd)
{
    if(!catEnable) return;
    if (serialPort->isOpen())
    {
        serialPort->write(cmd);
        if (!serialPort->waitForBytesWritten(TIMEOUT)) {
            qDebug() << "CAT Serial Port Write Error. " << serialPort->errorString();
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::portRead()
{
    if(!catEnable) return;
    QByteArray responseData = serialPort->readAll();
    while (serialPort->waitForReadyRead(TIMEOUT))
        responseData += serialPort->readAll();

    //qDebug() << "Read Port: " << responseData; //Отладочный вывод

    if (((responseData[0] == 'F') && (responseData[1] == 'A')) ||
        ((responseData[0] == 'I') && (responseData[1] == 'F')))
    {
        sscanf(responseData, "FA%11lu;", &freq);

        if (freq != old_freq) {
            emit cat_freq(freq);
            old_freq = freq;
        }

        band = freqToBand(freq);

        if (band != old_band) {
            emit cat_band(band);
            old_band = band;
        }
    }

    if ((responseData[0] == 'M') && (responseData[1] == 'D'))
    {
        sscanf(responseData, "MD%2d;", &mode);
        if (mode != old_mode) {
            convertMode(mode);
            old_mode = mode;
        }
    }
    responseData.clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::setBand(int band)
{
    switch(band) {
        case 2:
            sendCommand("BD00;"); //160M
            break;
        case 3:
            sendCommand("BD01;"); //80M
            break;
        case 5:
            sendCommand("BD02;"); //40M
            break;
        case 6:
            sendCommand("BD03;"); //30M
            break;
        case 7:
            sendCommand("BD04;"); //20M
            break;
        case 8:
            sendCommand("BD05;"); //17M
            break;
        case 9:
            sendCommand("BD06;"); //15M
            break;
        case 10:
            sendCommand("BD07;"); //12M
            break;
        case 11:
            sendCommand("BD08;"); //10M
            break;
        case 12:
            sendCommand("BD09;"); //6M
            break;
        case 13:
            sendCommand("BD10;"); //4M
            break;
        default:
            break;
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::setMode(int mode)
{
    switch(mode) {
        case 0: sendCommand("MD1;"); break; //LSB
        case 1: sendCommand("MD2;"); break; //USB
        case 2: sendCommand("MD3;"); break; //CW
        case 3: sendCommand("MD9;"); break; //FSK
        case 4: sendCommand("MD9;"); break; //FSK
        case 5: sendCommand("MD4;"); break; //FM
        case 6: sendCommand("MD5;"); break; //AM
        default: sendCommand("MD6;"); break; //FSK-R
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::setFreq(long f)
{
   char fr[15];

   if((f > 1800000)&&(f < 52000000))
   {
       sprintf(fr, "FA%11d;", f);
       sendCommand(fr);
   }
   freq = f;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int cat_Interface::freqToBand(long f)
{
    if (f >=   1800000 && f <=   2000000)  { band = 2;}         // 160m
    else if (f >=   3000000 && f <=   3800000)  { band = 3; }   // 80m
    else if (f >=   7000000 && f <=   7200000)  { band = 5; }   // 40m
    else if (f >=  10000000 && f <=  10150000)  { band = 6; }   // 30m
    else if (f >=  14000000 && f <=  14350000)  { band = 7; }   // 20m
    else if (f >=  18000000 && f <=  18168000)  { band = 8; }   // 17m
    else if (f >=  21000000 && f <=  21450000)  { band = 9; }   // 15m
    else if (f >=  24890000 && f <=  24990000)  { band = 10; }  // 12m
    else if (f >=  28000000 && f <=  29700000)  { band = 11; }  // 10m
    else if (f >=  50000000 && f <=  52000000)  { band = 12; }  // 6m
    else { band = 0; }                                          // out of range
    freq = f;
    return band;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::modeNameToMode(QString mode)
{
    QString md = mode.trimmed().toUpper(); // нормализуем строку

    if (md.isEmpty()) {
            return;
    }

    if (md == "SSB") {
        if (freq < 7300000) {
            sendCommand("MD1;"); // LSB
        } else {
            sendCommand("MD2;"); // USB
        }
    }
    else if (md == "LSB") {
        sendCommand("MD1;");
    }
    else if (md == "USB") {
        sendCommand("MD2;");
    }
    else if (md == "CW") {
        sendCommand("MD3;");
    }
    else if (md == "FSK" || md == "RTTY" || md == "FT8" || md == "FT4" || md == "MFSK") {
        sendCommand("MD9;");
    }
    else if (md == "FSK-R") {
        sendCommand("MD6;");
    }
    else if (md == "FM") {
        sendCommand("MD4;");
    }
    else if (md == "AM") {
        sendCommand("MD5;");
    }
    else {
        qWarning() << "Неизвестный режим:" << md;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::convertMode(int mode)
{
    int res;

    switch (mode)
    {
        case 0: res = 0; break; //None (setting failure)
        case 1: res = 0; break; //LSB
        case 2: res = 1; break; //USB
        case 3: res = 2; break; //CW
        case 4: res = 5; break; //FM
        case 5: res = 6; break; //AM
        case 6: res = 3; break; //FSK
        case 7: res = 2; break; //CW-R
        case 8: res = 0; break; //None (setting failure)
        case 9: res = 3; break; //FSK-R
        default: res = 0; break; //out of range
    };
    emit cat_mode(res);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::catSetBaudRate(int baud)
{
    switch (baud) {
    case 1200:
        serialPort->setBaudRate(QSerialPort::Baud1200);
        break;
    case 2400:
        serialPort->setBaudRate(QSerialPort::Baud2400);
        break;
    case 4800:
        serialPort->setBaudRate(QSerialPort::Baud4800);
        break;
    case 9600:
        serialPort->setBaudRate(QSerialPort::Baud9600);
        break;
    case 19200:
        serialPort->setBaudRate(QSerialPort::Baud19200);
        break;
    case 38400:
        serialPort->setBaudRate(QSerialPort::Baud38400);
        break;
    case 57600:
        serialPort->setBaudRate(QSerialPort::Baud57600);
        break;
    case 115200:
        serialPort->setBaudRate(QSerialPort::Baud115200);
        break;
    default:
        serialPort->setBaudRate(QSerialPort::Baud9600);
        break;
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::catSetParity(QString parity)
{
    if(parity == "No") serialPort->setParity(QSerialPort::NoParity);
    else if(parity == "Odd") serialPort->setParity(QSerialPort::OddParity);
    else if(parity == "Even") serialPort->setParity(QSerialPort::EvenParity);
    else if(parity == "Mark") serialPort->setParity(QSerialPort::MarkParity);
    else if(parity == "Space") serialPort->setParity(QSerialPort::SpaceParity);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::catSetStopBit(int stopbits)
{
    switch (stopbits) {
    case 1:
        serialPort->setStopBits(QSerialPort::OneStop);
        break;
    case 2:
        serialPort->setStopBits(QSerialPort::TwoStop);
        break;
    default:
        serialPort->setStopBits(QSerialPort::OneStop);
        break;
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::catSetDataBits(int databits)
{
    switch (databits) {
    case 7:
        serialPort->setDataBits(QSerialPort::Data7);
        break;
    case 8:
        serialPort->setDataBits(QSerialPort::Data8);
        break;
    default:
        serialPort->setDataBits(QSerialPort::Data8);
        break;
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::catSetFlowControl(QString flowcontrol)
{
    if(flowcontrol == "Disable") serialPort->setFlowControl(QSerialPort::NoFlowControl);
    else if(flowcontrol == "Hardware") serialPort->setFlowControl(QSerialPort::HardwareControl);
    else if(flowcontrol == "Software") serialPort->setFlowControl(QSerialPort::SoftwareControl);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::setInterval(int interval)
{
    catTimer->setInterval(interval);
    //qDebug() << "CAT Timer Interval: " << interval;
}
//------------------------------------------------------------------------------------------------------------------------------------------
/*
byte rig_group = (digitalRead(RIG_JMP_1) * 2) + digitalRead(RIG_JMP_0);

    switch (rig_group) {
        case 3: //TS-590, TS-890, TS480, TS-2000 (NO jumpers)
            frq_len_a = 11; //The number of digits representing the frequency ('FA').
            frq_len_b = 11; //The number of digits representing the frequency ('FB').
            break;
        case 2: //FT-991A, FTDX-10, FTDX-101 (only RIG_JMP_0 in place)
            frq_len_a = 9;
            frq_len_b = 9;
            break;
        case 1: //FTDX5000, FTDX9000 (only RIG_JMP_1 in place)
            frq_len_a = 8;
            frq_len_b = 8;
            break;
        case 0: //FT-950, FT-2000, FTDX-3000, FTDX-1200 (BOTH jumpers in place)
            frq_len_a = 8;
            frq_len_b = 11;
            break;
    }
*/
//------------------------------------------------------------------------------------------------------------------------------------------

















