#include "cat_interface.h"
#include <QDebug>
#include <string>

#define TIMEOUT 100


cat_Interface::cat_Interface(bool catEnable, QObject *parent)
    : QObject{parent}
{
    this->catEnable = catEnable;
    serialPort = new QSerialPort;
    connect(serialPort, &QSerialPort::readyRead, this, &cat_Interface::portRead); //Соединяем сигнал
    catTimer = new QTimer(this);
    catTimer->setInterval(2000);
    connect(catTimer, &QTimer::timeout, this, [=]() {
        if(catEnable)
        {
            sendCommand("FA;");
            sendCommand("MD;");
        }
    });
}
//------------------------------------------------------------------------------------------------------------------------------------------

cat_Interface::~cat_Interface()
{
    if(serialPort->isOpen()) serialPort->close();
    delete serialPort;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::handleError(QSerialPort::SerialPortError error)
{
   qDebug() << "Ошибка: " << error;
   if(serialPort->isOpen()) serialPort->close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::openSerial(QString port)
{
    serialPort->setPortName(port);
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Ошибка, Не удалось подключится к порту";
        catEnable = false;
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::sendCommand(const char *cmd)
{
    serialPort->write(cmd); //Запрос состояния трансивера

    if(serialPort->waitForBytesWritten(TIMEOUT)) {
        //qDebug() << "CAT command: " << cmd;
    }
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
        case 0:
            sendCommand("MD1;"); //LSB
            break;
        case 1:
            sendCommand("MD2;"); //USB
            break;
        case 2:
            sendCommand("MD3;"); //CW
            break;
        case 3 :
            sendCommand("MD9;"); //FSK
            break;
        case 4 :
            sendCommand("MD9;"); //FSK
            break;
        case 5:
            sendCommand("MD4;"); //FM
            break;
        case 6:
            sendCommand("MD5;"); //AM
            break;
        default:
            sendCommand("MD6;"); //FSK-R
            break;
    };
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::setFreq(long freq)
{
   char fr[15];

   if((freq > 1810000)&&(freq < 52000000))
   {
       sprintf(fr, "FA%11d;", freq);
       sendCommand(fr);
   }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::portRead()
{
    QByteArray responseData = serialPort->readAll();
    while (serialPort->waitForReadyRead(TIMEOUT)) /*qDebug() << "Trx answer:" << QString::fromUtf8(responseData)*/;

    if((responseData[0] == 'F')&&(responseData[1] == 'A')) {
        sscanf(responseData, "FA%11lu;", &freq);
        band = freqToBand(freq);

        if(freq != old_freq) {
            emit cat_freq(freq);
            //qDebug() << "Freq: " << freq;
            old_freq = freq;
        }
        if(band != old_band) {
            emit cat_band(band);
            //qDebug() << "Band: " << band;
            old_band = band;
        }
    }
    if((responseData[0] == 'M')&&(responseData[1] == 'D')) {
        sscanf(responseData, "MD%2d;", &mode);
        if(mode != old_mode) {
            convertMode(mode);
            //qDebug() << "Mode: " << mode;
            old_mode = mode;
        }
    }
    responseData.clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

int cat_Interface::freqToBand(long freq)
{
    if (freq >=   1810000 && freq <=   2000000)  { band = 2;}         // 160m
    else if (freq >=   3500000 && freq <=   3800000)  { band = 3; }   // 80m
    else if (freq >=   7000000 && freq <=   7200000)  { band = 5; }   // 40m
    else if (freq >=  10100000 && freq <=  10150000)  { band = 6; }   // 30m
    else if (freq >=  14000000 && freq <=  14350000)  { band = 7; }   // 20m
    else if (freq >=  18068000 && freq <=  18168000)  { band = 8; }   // 17m
    else if (freq >=  21000000 && freq <=  21450000)  { band = 9; }   // 15m
    else if (freq >=  24890000 && freq <=  24990000)  { band = 10; }  // 12m
    else if (freq >=  28000000 && freq <=  29700000)  { band = 11; }  // 10m
    else if (freq >=  50000000 && freq <=  52000000)  { band = 12; }  // 6m
    else { band = 0; }                                                // out of range
    return band;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void cat_Interface::convertMode(int mode)
{
    int res;

    if(mode == 0) { res = 0; }       //None (setting failure)
    else if(mode == 1) { res = 0; }  //LSB
    else if(mode == 2) { res = 1; }  //USB
    else if(mode == 3) { res = 2; }  //CW
    else if(mode == 4) { res = 5; }  //FM
    else if(mode == 5) { res = 6; }  //AM
    else if(mode == 6) { res = 3; }  //FSK
    else if(mode == 7) { res = 2; }  //CW-R
    else if(mode == 8) { res = 0; }  //None (setting failure)
    else if(mode == 9) { res = 3; }  //FSK-R
    else { res = 0; }                //out of range
    emit cat_mode(res);
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


















