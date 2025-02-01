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
            //rigRequesting(); //вызываем функцию чтобы порт точно закрылся
            serialPort.close();
            portOpened=false;

            ui->freqLabel->setText("Частота:");
            ui->modeLabel->setText("Модуляция:");
        }

    });



    setWindowTitle("CAT для Kenwood");
    connect(ui->saveCATsettings, &QPushButton::clicked, this, &loggerCAT::saveCATset);
    connect(ui->closeCATwindow, &QPushButton::clicked, this, &loggerCAT::hide);

    //изменяем настройки при изменении их пользователем в интерфейсе
    /*connect(ui->CATspeed, &QComboBox::currentIndexChanged, this, &loggerCAT::setupPort);
    connect(ui->CATport, SIGNAL(currentIndexChanged), this, SLOT(setupPort()));
    connect(ui->CATstopbits, SIGNAL(currentIndexChanged), this, SLOT(setupPort()));
    connect(ui->flowctrl, SIGNAL(currentIndexChanged), this, SLOT(setupPort()));
*/
    connect(ui->intvlEdit, &QLineEdit::editingFinished, this, [=] () {
                    requestTimer->setInterval(ui->intvlEdit->text().toInt());
                });

    connect(ui->setupButton, &QPushButton::clicked, this, &loggerCAT::setupPort);
    connect(this, &loggerCAT::portErr, this, [=] (){
        //requestTimer->stop();
        ui->useLoggerCAT->setChecked(false);
        QMessageBox::warning(this, "Ошибка", "Не удалось подключится к порту");
        portOpened = false;
        //return;
    });

}

loggerCAT::~loggerCAT()
{
    delete ui;
}





QList<QString> loggerCAT::rigRequesting()
{
    if (!portOpened) {

        setupPort();

        if (!serialPort.open(QIODevice::ReadWrite)) {
        // если подключится не получится, то покажем сообщение с ошибкой
            emit portErr();
    }
        else portOpened = true;
    }


    // вместо отдельного опроса модуляции и частоты (с определением выбранного гетеродина по изменению частоты)
    // можно попробовать использовать команды IF - выводится ОДНА частота и режим
    // или AI - автоматическое информирование об изменении параметров



    /*
    // способ с раздельным опросом
    serialPort.write("FA;");
    serialPort.waitForBytesWritten();

    QByteArray rigFreq;
    while (serialPort.waitForReadyRead(50)) {

        rigFreq.append(serialPort.readAll());
    }
    ui->CATreceived->append(rigFreq);

    serialPort.write("MD;");
    serialPort.waitForBytesWritten();

    QByteArray rigMode;
    while (serialPort.waitForReadyRead(50)) {

        rigFreq.append(serialPort.readAll());
    }
    ui->CATreceived->append(rigMode);
    */

    // способ с постоянным опросом через IF
    serialPort.write("IF;");
    serialPort.waitForBytesWritten(); // опрашиваем трансивер

    QByteArray rigAnswer; // читаем ответ
    while (serialPort.waitForReadyRead(590)) {

        rigAnswer.append(serialPort.readAll());
    }
    QString answ=rigAnswer; // информация о трансивере, включая rit, xit, mode и др.
    QString rigFreqHz = QString::number(answ.mid(2, 11).toInt()); //символы с 3 по 12 - это частота. эта переменная для поля в mainwindow
    QString rigFreq = QString::number(rigFreqHz.toFloat()/1000000); //узнаем частоту и переводим в МГц. а эта переменная для контрольного поля в этом окне
    QString rigMode=answ.mid(29, 1); //узнаем модуляцию, здесь будет число от 1 до 9

    QVector<QString> modes={"no selection", "SSB (LSB)", "SSB (USB)", "CW", "FM", "AM", "FSK", "CW", "no selection", "FSK"};
    // список соответсвия номера и модуляции, будем узнавать модуляцию по индексу. CW-R и FSK-R записаны без R
    QString rigModeName=modes.at(rigMode.toInt());


    ui->freqLabel->setText("Частота: " + rigFreq);
    ui->modeLabel->setText("Модуляция: " + rigModeName);

    if (portOpened & !ui->useLoggerCAT->isChecked()){
        serialPort.close(); // если убрать галку с чекбокса во время выполнения след. строк, то он останется открытым?!
        portOpened=false;

    }


    QList<QString> info;
    info.append(rigFreqHz);
    info.append(rigModeName); //наверно можно было эти переменные записать в public в хедере, но я поздновато это понял
    return info;
}
    void loggerCAT::setupPort()
    {
        // тут идут настройки порта. data bits (по умолчанию 8) я не устанавливал, т.к. вроде у всех Kenwood 8 data bits
        serialPort.setPortName(this->ui->CATport->currentText());
        qint32 baud=this->ui->CATspeed->currentText().toInt();

        if(this->ui->flowctrl->currentText()=="Нет"){
            serialPort.setFlowControl(QSerialPort::NoFlowControl);
        }
        else{
            serialPort.setFlowControl(QSerialPort::HardwareControl);
        }

        serialPort.setBaudRate(baud);
        int32_t stopbits=this->ui->CATstopbits->currentText().toInt();
        switch (stopbits)
        {
        case 1:
            serialPort.setStopBits(QSerialPort::OneStop);
            break;
        case 2:
            serialPort.setStopBits(QSerialPort::TwoStop);
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

