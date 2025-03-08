#include "loggercat.h"
#include "ui_loggercat.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <qfileinfo.h>
#include <qmessagebox.h>



loggerCAT::loggerCAT(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::loggerCAT)
{

    ui->setupUi(this);
    serialPort = new QSerialPort(this);

    ui->freqLabel->setText("Частота:");
    ui->modeLabel->setText("Модуляция:");

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->CATport->addItem(serialPortInfo.portName());
    }

    openPath(QCoreApplication::applicationDirPath() + "/settings.ini");

    QTimer *requestTimer = new QTimer(this);
    requestTimer->setInterval(CATinterval);

    connect(requestTimer, SIGNAL(timeout()), this, SLOT(rigTmrSig()));
    connect(ui->useLoggerCAT, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            requestTimer->start();
        } else {
            requestTimer->stop();
            serialPort->close();

            ui->freqLabel->setText("Частота:");
            ui->modeLabel->setText("Модуляция:");
        }

    });



    setWindowTitle("CAT для Kenwood");
    connect(ui->saveCATsettings, &QPushButton::clicked, this, &loggerCAT::saveCATset);
    connect(ui->closeCATwindow, &QPushButton::clicked, this, &loggerCAT::hide);


    connect(ui->intvlEdit, &QLineEdit::editingFinished, this, [=] () {
                    requestTimer->setInterval(ui->intvlEdit->text().toInt());
                });

    connect(ui->setupButton, &QPushButton::clicked, this, &loggerCAT::setupPort);
    connect(this, &loggerCAT::portErr, this, [=] (){
        ui->useLoggerCAT->setChecked(false);
        QMessageBox::warning(this, "Ошибка", "Не удалось подключится к порту");
    });
    connect(this->serialPort, SIGNAL(readyRead()), this, SLOT(reading()));

}

loggerCAT::~loggerCAT()
{
    delete ui;
}





void loggerCAT::rigRequesting()
{
    if (!serialPort->isOpen()) {

        setupPort();

        if (!serialPort->open(QIODevice::ReadWrite)) emit portErr(); // если подключится не получится, то покажем сообщение с ошибкой
    }


    // вместо отдельного опроса модуляции и частоты (с определением выбранного гетеродина по изменению частоты)
    // можно попробовать использовать команды IF - выводится ОДНА частота и режим
    // или AI - автоматическое информирование об изменении параметров



    /*
    // способ с раздельным опросом
    serialPort->write("FA;");
    serialPort->waitForBytesWritten();

    QByteArray rigFreq;
    while (serialPort->waitForReadyRead(50)) {

        rigFreq.append(serialPort->readAll());
    }
    ui->CATreceived->append(rigFreq);

    serialPort->write("MD;");
    serialPort->waitForBytesWritten();

    QByteArray rigMode;
    while (serialPort->waitForReadyRead(50)) {

        rigFreq.append(serialPort->readAll());
    }
    ui->CATreceived->append(rigMode);
    */

    // способ с постоянным опросом через IF
    serialPort->write("IF;");
    serialPort->waitForBytesWritten(); // опрашиваем трансивер



    if (serialPort->isOpen() & !ui->useLoggerCAT->isChecked()){
        serialPort->close();
    }



}
    void loggerCAT::setupPort()
    {
        // тут идут настройки порта. data bits (по умолчанию 8) я не устанавливал, т.к. вроде у всех Kenwood 8 data bits
        serialPort->setPortName(this->ui->CATport->currentText());
        qint32 baud=this->ui->CATspeed->currentText().toInt();

        if(this->ui->flowctrl->currentText()=="Нет"){
            serialPort->setFlowControl(QSerialPort::NoFlowControl);
        }
        else{
            serialPort->setFlowControl(QSerialPort::HardwareControl);
        }

        serialPort->setBaudRate(baud);
        int32_t stopbits=this->ui->CATstopbits->currentText().toInt();
        switch (stopbits)
        {
        case 1:
            serialPort->setStopBits(QSerialPort::OneStop);
            break;
        case 2:
            serialPort->setStopBits(QSerialPort::TwoStop);
            break;

        }
    }



void loggerCAT::rigTmrSig()
{
    if(ui->useLoggerCAT->isChecked()) emit rigupd();

}

//сохранение настроек по аналогии с settings в тот же файл в группе CAT
void loggerCAT::openPath(QString path) {
    this->path = path;
    QFileInfo portset(path);
    if (!portset.exists()) {
        createSetFile();
    }

    qs = new QSettings(path, QSettings::IniFormat);
    setRead();
}


void loggerCAT::createSetFile()
{
    QFile newFile(path);
    newFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&newFile);
    stream << "[API]" << Qt::endl;
    stream << "token =" << Qt::endl;
    stream << Qt::endl;
    stream << "[UDP]" << Qt::endl;
    stream << "enable = 1" << Qt::endl;
    stream << "port = 2237" << Qt::endl;
    stream << Qt::endl;
    stream << "[FLRIG]" << Qt::endl;
    stream << "host = 127.0.0.1" << Qt::endl;
    stream << "port = 12345" << Qt::endl;
    stream << "period = 500" << Qt::endl;
    stream << Qt::endl;
    stream << "[QRZRU]" << Qt::endl;
    stream << "enable = 0" << Qt::endl;
    stream << "login = " << Qt::endl;
    stream << "password = " << Qt::endl;
    stream << Qt::endl;
    stream << "[VIEW]" << Qt::endl;
    stream << "fontsize = 10" << Qt::endl;
    stream << "darktheime = true" << Qt::endl;
    stream << Qt::endl;
    stream << "[FORM]" << Qt::endl;
    stream << "band = 20M" << Qt::endl;
    stream << "mode = SSB (USB)" << Qt::endl;
    stream << "freq = " << Qt::endl;
    stream << "callsign = " << Qt::endl;
    stream << "operator = " << Qt::endl;
    stream << "RDA = " << Qt::endl;
    stream << "LOC = " << Qt::endl;
    stream << "rst_send = " << Qt::endl;
    stream << "rst_rcvd = " << Qt::endl;
    stream << "[CAT]" << Qt::endl;
    stream << "portName = " << Qt::endl;
    stream << "baud = 4800" << Qt::endl;
    stream << "stopBits = 1" << Qt::endl;
    stream << "flowControl = 0" << Qt::endl;
    stream << "CATinterval = 400" << Qt::endl;
    newFile.close();
}
void loggerCAT::setRead()
{
    qs->beginGroup("CAT");
    ui->CATport->setCurrentText(qs->value("portName").toString());
    ui->CATspeed->setCurrentText(qs->value("baud").toString());
    ui->CATstopbits->setCurrentText(qs->value("stopBits").toString());
    if (qs->value("flowControl").toBool()) ui->flowctrl->setCurrentText("Аппаратное");
    else ui->flowctrl->setCurrentText("Нет");
    CATinterval = qs->value("CATinterval").toInt();
    ui->intvlEdit->setText(QString::number(CATinterval));
    qs->endGroup();

}
void loggerCAT::saveCATset()
{
    qs->beginGroup("CAT");
    qs->setValue("portName", ui->CATport->currentText());
    qs->setValue("baud", ui->CATspeed->currentText());
    qs->setValue("stopBits", ui->CATstopbits->currentText());
    if (ui->flowctrl->currentText()=="Нет") qs->setValue("flowControl", false);
    else qs->setValue("flowControl", true);
    qs->setValue("CATinterval", ui->intvlEdit->text());
    qs->endGroup();

    qs->sync();
    emit CATSettingsChanged();
}


void loggerCAT::reading()
{
    rigAnswer.append(serialPort->readAll());
    /* readAll и readLine почему-то читают только по 8 байт (не у меня одного так, похоже, что так и должно быть.
    или дело в объеме буфера), поэтому ждем, пока наберутся 38 байт ответа. (readyRead генерируется каждые "8 байт")*/
    if (rigAnswer.count()>=38)
    //когда полностью приняли ответ, смотрим частоту. Вместо readAll можно было использовать readLine(char *data, 38-rigAnswer.count())
    //т.е. читаем оставшуюся часть ответа, чтобы не прочитать "в никуда" начало следующего ответа (такое может случится
    //при малом интервале опроса, когда может не дойти перевод на новую строку или другие символы), к тому же
    //сигнал таймера rigUpd() и updated(), генерируемый при успешномм прочтении частоты, никак не синхронизированы
    //и, когда в ответе потеряются какие нибудь байты, и он не пройдет следующее условие, может отправиться
    //еще один ответ с трансивера в частично заполненный буфер
    {
    answ = rigAnswer; // информация о трансивере, включая rit, xit, mode и др. 38 символов

    rigAnswer.clear();

    if (answ.count()==38 && answ.mid(0, 2)=="IF" && answ.mid(37, 1)==";")
    //подходят только ответы длиной 38 байт(символов) вида "IF<35 байт>;"
    //чтобы частично решить первую проблему и проблему с потерей байтов в середине ответа (при тестах с моим TS-570D
    //почему-то обычно терялись пробелы в середине сообщения, а информация доходила целой) можно было поступить так:
    //находить "F". следующие 11 байт после нее это частота. Находить ";". 8-й символ левее ";" - это rigMode_.
    //Но при интервале опроса >600 мс все работает нормально. Аппаратное управление потоком нет возможности проверить, может там все работает хорошо и так.
    {
    //qDebug("ok " + answ);
    unsigned long long rigFreqHz = answ.mid(2, 11).toULongLong(); //символы с 3 по 12 - это частота. эта переменная для поля в mainwindow
    QString rigFreq_ = QString::number((double) rigFreqHz/1000000, 'f', 6); //узнаем частоту и переводим в МГц. а эта переменная для контрольного поля в этом окне
    int rigMode_=answ.mid(29, 1).toInt(); //узнаем модуляцию, здесь будет число от 1 до 9

    QVector<QString> modes={"no selection", "SSB (LSB)", "SSB (USB)", "CW", "FM", "AM", "FSK", "CW", "no selection", "FSK"};
    // список соответсвия номера и модуляции, будем узнавать модуляцию по индексу. CW-R и FSK-R записаны без R
    QString rigModeName=modes.at(rigMode_);

    ui->freqLabel->setText("Частота: " + rigFreq_);
    ui->modeLabel->setText("Модуляция: " + rigModeName);
    rigFreq = rigFreqHz;
    rigMode = rigModeName;
    emit updated();
    }
    }
    //else qDebug(rigAnswer);

}
