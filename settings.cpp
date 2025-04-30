#include "settings.h"
#include "ui_settings.h"
#include <QDebug>


Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Настройки программы");

    //Определение списка доступных портов
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        //qDebug() << "Available Ports: " << info.portName();
        ui->SerialPortComboBox->addItem(info.portName());
    }

    ui->accessToken->setEchoMode(QLineEdit::Password);
    ui->LogRadioAccessToken->setEchoMode(QLineEdit::Password);
    ui->qrzruPassword->setEchoMode(QLineEdit::Password);

    logradio = new APILogRadio(logRadioAccessToken);
    connect(logradio, SIGNAL(checked(int,QString)), this, SLOT(checked(int,QString)));
    connect(logradio, SIGNAL(received(QString, QString, QString, QString, QString, QString)), this, SLOT(received(QString, QString, QString, QString, QString, QString)));

    connect(ui->saveButton, &QPushButton::clicked, this, &Settings::save);
    connect(ui->closeButton, &QPushButton::clicked, this, &Settings::hide);
    connect(ui->getLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::getLogRadioToken);
    connect(ui->checkLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::checkLogRadioToken);
    openPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/settings.ini");
    premium = "0";
}
//------------------------------------------------------------------------------------------------------------------------------------------

Settings::~Settings() {
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::openPath(QString path) {
    this->path = path;
    QFileInfo info(path);
    if (!info.exists()) {
        createDefaultFile();
    }
    qs = new QSettings(path, QSettings::IniFormat);
    read();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::read() {
    QString str;
    qs->beginGroup("API");
    str = qs->value("token", "").toString();
    useCallbook = qs->value("callbook", false).toBool();
    accessToken = EncryptToken(str);
    qs->endGroup();
    qs->beginGroup("APILOGRADIORU");
    str = qs->value("token", "").toString();
    logRadioAccessToken = EncryptToken(str);
    qs->endGroup();
    qs->beginGroup("UDP");
    udpServerEnable = qs->value("enable", true).toBool();
    udpServerPort = qs->value("port", 2240).toInt();
    udpClientEnable = qs->value("enable_retransl", false).toBool();
    udpClientPort = qs->value("port_retransl", 2244).toInt();
    qs->endGroup();
    qs->beginGroup("CAT");
    catEnable = qs->value("enable", false).toBool();
    catInterval = qs->value("interval", 1500).toInt();
    trxType = qs->value("trx", "").toString();
    serialPort = qs->value("port", "").toString();
    serialPortBaud = qs->value("baud", "57600").toString();
    serialPortDataBits = qs->value("databits", "8").toString();
    serialPortStopBit = qs->value("stopbit", "1").toString();
    serialPortParity = qs->value("parity", "Нет").toString();
    serialPortFlowControl = qs->value("flowctrl", "Отключено").toString();
    qs->endGroup();
    qs->beginGroup("FLRIG");
    flrigHost = QHostAddress(qs->value("host", "127.0.0.1").toString());
    flrigPort = qs->value("port", 12345).toInt();
    flrigPeriod = qs->value("period", 500).toInt();
    qs->endGroup();
    qs->beginGroup("QRZRU");
    enableQrzruCallbook = qs->value("enable", true).toBool();
    QrzruLogin = qs->value("login", "").toString();
    QrzruPassword = qs->value("password", "").toString();
    qs->endGroup();
    qs->beginGroup("VIEW");
    fontSize = qs->value("fontsize", 10).toInt();
    darkTheime = qs->value("darktheime", false).toBool();
    qs->endGroup();
    qs->beginGroup("FORM");
    lastBand = qs->value("band", "").toString();
    lastMode = qs->value("mode", "").toString();
    lastFrequence = qs->value("freq", "").toString();
    lastCallsign = qs->value("callsign", "").toInt();
    lastOperator = qs->value("operator", "").toInt();
    lastRDA = qs->value("RDA", "").toString();
    lastLocator = qs->value("LOC", "").toString();
    lastRST_SENT = qs->value("rst_send", "").toString();
    lastRST_RCVD = qs->value("rst_rcvd", "").toString();
    qs->endGroup();

    display();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::display() {
    ui->accessToken->setText(accessToken);
    ui->LogRadioAccessToken->setText(logRadioAccessToken);
    ui->udpServerEnableCheckbox->setChecked(udpServerEnable);
    ui->udpServerPort->setValue(udpServerPort);
    ui->udpClientEnableCheckbox->setChecked(udpClientEnable);
    ui->udpClientPort->setValue(udpClientPort);
    ui->flrigHost->setText(flrigHost.toString());
    ui->flrigPort->setValue(flrigPort);
    ui->flrigPeriod->setValue(flrigPeriod);
    ui->qrzruEnable->setChecked(enableQrzruCallbook);
    ui->qrzruLogin->setText(QrzruLogin);
    ui->qrzruPassword->setText(QrzruPassword);
    ui->fontSize->setValue(fontSize);
    ui->darkTheimeCheckBox->setChecked(darkTheime);
    ui->CallbookCheckBox->setChecked(useCallbook);
    ui->EnableCATcheckBox->setChecked(catEnable);
    ui->IntervalSpinBox->setValue(catInterval);
    ui->TRXTypeComboBox->setCurrentText(trxType);
    ui->SerialPortComboBox->setCurrentText(serialPort);
    ui->SerialPortBaudComboBox->setCurrentText(serialPortBaud);
    ui->SerialPortDataBitsComboBox->setCurrentText(serialPortDataBits);
    ui->SerialPortStopBitComboBox->setCurrentText(serialPortStopBit);
    ui->SerialPortParityComboBox->setCurrentText(serialPortParity);
    ui->SerialPortFlowControlComboBox->setCurrentText(serialPortFlowControl);
    ui->tabWidget->setCurrentIndex(0);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::createDefaultFile() {
    QFile newFile(path);
    newFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&newFile);
    stream << "[API]" << Qt::endl;
    stream << "token =" << Qt::endl;
    stream << "callbook =" << Qt::endl;
    stream << Qt::endl;
    stream << "[APILOGRADIORU]" << Qt::endl;
    stream << "token =" << Qt::endl;
    stream << Qt::endl;
    stream << "[UDP]" << Qt::endl;
    stream << "enable = 0" << Qt::endl;
    stream << "port = " << Qt::endl;
    stream << "enable_retransl = 0" << Qt::endl;
    stream << "port_retransl = " << Qt::endl;
    stream << Qt::endl;
    stream << "[CAT]" << Qt::endl;
    stream << "enable =" << Qt::endl;
    stream << "interval =" << Qt::endl;
    stream << "trx =" << Qt::endl;
    stream << "port =" << Qt::endl;
    stream << "baud =" << Qt::endl;
    stream << "databits =" << Qt::endl;
    stream << "stopbit =" << Qt::endl;
    stream << "parity =" << Qt::endl;
    stream << "flowctrl =" << Qt::endl;
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
    newFile.close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::save() {
    qs->beginGroup("API");
    qs->setValue("token", EncryptToken(ui->accessToken->text()));
    qs->setValue("callbook", ui->CallbookCheckBox->isChecked() ? 1 : 0);
    qs->endGroup();
    qs->beginGroup("APILOGRADIORU");
    qs->setValue("token", EncryptToken(ui->LogRadioAccessToken->text()));
    qs->endGroup();
    qs->beginGroup("UDP");
    qs->setValue("enable", ui->udpServerEnableCheckbox->isChecked() ? 1 : 0);
    qs->setValue("port", ui->udpServerPort->value());
    qs->setValue("enable_retransl", ui->udpClientEnableCheckbox->isChecked() ? 1 : 0);
    qs->setValue("port_retransl", ui->udpClientPort->value());
    qs->endGroup();
    qs->beginGroup("CAT");
    qs->setValue("enable", ui->EnableCATcheckBox->isChecked() ? 1 : 0);
    if (ui->IntervalSpinBox->value() >= 20) qs->setValue("interval", ui->IntervalSpinBox->value());
    else qs->setValue("interval", 20); // при интервале <20 команда будет отправляться в порт, пока прошлая еще не отправилась полностью
    qs->setValue("trx", ui->TRXTypeComboBox->currentText());
    qs->setValue("port", ui->SerialPortComboBox->currentText());
    qs->setValue("baud", ui->SerialPortBaudComboBox->currentText());
    qs->setValue("databits", ui->SerialPortDataBitsComboBox->currentText());
    qs->setValue("stopbit", ui->SerialPortStopBitComboBox->currentText());
    qs->setValue("parity", ui->SerialPortParityComboBox->currentText());
    qs->setValue("flowctrl", ui->SerialPortFlowControlComboBox->currentText());
    qs->endGroup();
    qs->beginGroup("FLRIG");
    qs->setValue("host", ui->flrigHost->text());
    qs->setValue("port", ui->flrigPort->value());
    qs->setValue("period", ui->flrigPeriod->value());
    qs->endGroup();
    qs->beginGroup("QRZRU");
    qs->setValue("enable", ui->qrzruEnable->isChecked() ? 1 : 0);
    qs->setValue("login", ui->qrzruLogin->text());
    qs->setValue("password", ui->qrzruPassword->text());
    qs->endGroup();
    qs->beginGroup("VIEW");
    qs->setValue("fontsize", ui->fontSize->value());
    qs->setValue("darktheime", ui->darkTheimeCheckBox->isChecked() ? 1 : 0);
    qs->endGroup();
    qs->sync();
    emit SettingsChanged();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::saveForm()
{
    qs->beginGroup("FORM");
    qs->setValue("band", lastBand);
    qs->setValue("mode", lastMode);
    qs->setValue("freq", lastFrequence);
    qs->setValue("callsign", lastCallsign);
    qs->setValue("operator", lastOperator);
    qs->setValue("RDA", lastRDA);
    qs->setValue("LOC", lastLocator);
    qs->setValue("rst_send", lastRST_SENT);
    qs->setValue("rst_rcvd", lastRST_RCVD);
    qs->endGroup();
    qs->sync();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString Settings::EncryptToken(QString data)
{
    int ln = data.length();
    QString sd = genSalt(data);

    QString result = "";
    for (int i = 0; i < ln; i++)
    {
         result.append(QString(QChar(data[i]).unicode()^QChar(sd[i]).unicode()));
    }
    return result;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString Settings::genSalt(QString data)
{
    int dataLength = data.length();
    QString Salt;
    QByteArray UID = QSysInfo::machineUniqueId();

    for(int i = 0; i < dataLength; ++i)
    {
        int index = i % UID.length();
        QChar nextUIDChar = UID.at(index);
        Salt.append(nextUIDChar);
     }
    return Salt;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::getLogRadioToken()
{
   logradio->getToken();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::checkLogRadioToken()
{
    logradio->checkToken();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::getUserInfo(QStringList data)
{
   QString registered = QDateTime::fromSecsSinceEpoch(data.at(5).toLong()).toString("dd.MM.yyyy");
   QString last_activity = QDateTime::fromSecsSinceEpoch(data.at(2).toLong()).toString("dd.MM.yyyy в hh:mm");
   QString premium_time = QDateTime::fromSecsSinceEpoch(data.at(4).toLong()).toString("dd.MM.yyyy");
   ui->registered_label->setText("Дата регистрации: " + registered);
   ui->last_activit_label->setText("Последняя активность: " + last_activity);
   premium = data.at(3);

   if(premium == "1") {
       ui->userStatusLabel->setText("Подписка Premium: Активна до " + premium_time);
       ui->CallbookCheckBox->setEnabled(true);
   }
   else ui->userStatusLabel->setText("Подписка Premium: Не активна");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::checked(int code, QString message)
{
    QMessageBox::information(0, "LogRadio.ru", message + "\nКод ответа: " + QString::number(code), QMessageBox::Ok);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::received(QString access_token, QString confirmation_key, QString confirmation_after, QString confirmation_before, QString valid_after, QString valid_before)
{
    if(access_token.length() > 0) {
    QMessageBox::information(0, "LogRadio.ru",
                                "Введите ключ подтверждения в разделе Токены API на LogRadio.ru\nКлюч подтверждения: " + confirmation_key + "\n" +
                                "Интервал подтверждения: от " + confirmation_after + " до " + confirmation_before + "\n" +
                                "Срок действия токена: от " + valid_after + " до " + valid_before, QMessageBox::Ok);
    ui->LogRadioAccessToken->setText(access_token);
    } else {
        QMessageBox::critical(0, "LogRadio.ru", "Не удалось получить токен.", QMessageBox::Ok);
    }

}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::on_CallbookCheckBox_toggled(bool checked)
{
    if(checked) {
        ui->qrzruEnable->setChecked(Qt::Unchecked);
        useCallbook = true;
    } else {
        ui->qrzruEnable->setChecked(Qt::Checked);
        useCallbook = false;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::on_qrzruEnable_toggled(bool checked)
{
    if(checked) {
        ui->CallbookCheckBox->setChecked(Qt::Unchecked);
        useCallbook = false;
    } else {
        ui->CallbookCheckBox->setChecked(Qt::Checked);
        useCallbook = true;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------


