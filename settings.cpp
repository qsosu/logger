#include "settings.h"
#include "ui_settings.h"
#include <QDebug>


Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    setWindowTitle("Настройки программы");
    ui->accessToken->setEchoMode(QLineEdit::Password);
    ui->LogRadioAccessToken->setEchoMode(QLineEdit::Password);
    ui->qrzruPassword->setEchoMode(QLineEdit::Password);

    connect(ui->saveButton, &QPushButton::clicked, this, &Settings::save);
    connect(ui->closeButton, &QPushButton::clicked, this, &Settings::hide);
    connect(ui->getLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::getLogRadioToken);
    connect(ui->checkLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::checkLogRadioToken);
    //openPath(QCoreApplication::applicationDirPath() + "/settings.ini");
    openPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/settings.ini");

}

Settings::~Settings() {
    delete ui;
}

void Settings::openPath(QString path) {
    this->path = path;
    QFileInfo info(path);
    if (!info.exists()) {
        createDefaultFile();
    }
    qs = new QSettings(path, QSettings::IniFormat);
    read();
}

void Settings::read() {
    QString str;
    qs->beginGroup("API");
    str = qs->value("token", "").toString();
    accessToken = EncryptToken(str);
    qs->endGroup();
    qs->beginGroup("APILOGRADIORU");
    str = qs->value("token", "").toString();
    logRadioAccessToken = EncryptToken(str);
    qs->endGroup();
    qs->beginGroup("UDP");
    udpServerEnable = qs->value("enable", true).toBool();
    udpServerPort = qs->value("port", 2237).toInt();
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

void Settings::display() {
    ui->accessToken->setText(accessToken);
    ui->LogRadioAccessToken->setText(logRadioAccessToken);
    ui->udpServerEnableCheckbox->setChecked(udpServerEnable);
    ui->udpServerPort->setValue(udpServerPort);
    ui->flrigHost->setText(flrigHost.toString());
    ui->flrigPort->setValue(flrigPort);
    ui->flrigPeriod->setValue(flrigPeriod);
    ui->qrzruEnable->setChecked(enableQrzruCallbook);
    ui->qrzruLogin->setText(QrzruLogin);
    ui->qrzruPassword->setText(QrzruPassword);
    ui->fontSize->setValue(fontSize);
    ui->darkTheimeCheckBox->setChecked(darkTheime);

    logradio = new APILogRadio(logRadioAccessToken);
    connect(logradio, SIGNAL(checked(int,QString)), this, SLOT(checked(int,QString)));
    connect(logradio, SIGNAL(received(QString, QString, QString, QString, QString, QString)), this, SLOT(received(QString, QString, QString, QString, QString, QString)));
}

void Settings::createDefaultFile() {
    QFile newFile(path);
    newFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&newFile);
    stream << "[API]" << Qt::endl;
    stream << "token =" << Qt::endl;
    stream << Qt::endl;
    stream << "[APILOGRADIORU]" << Qt::endl;
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
    newFile.close();
}

void Settings::save() {
    qs->beginGroup("API");
    qs->setValue("token", EncryptToken(ui->accessToken->text()));
    qs->endGroup();
    qs->beginGroup("APILOGRADIORU");
    qs->setValue("token", EncryptToken(ui->LogRadioAccessToken->text()));
    qs->endGroup();
    qs->beginGroup("UDP");
    qs->setValue("enable", ui->udpServerEnableCheckbox->isChecked() ? 1 : 0);
    qs->setValue("port", ui->udpServerPort->value());
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


void Settings::getLogRadioToken()
{
   logradio->getToken();
}

void Settings::checkLogRadioToken()
{
    logradio->checkToken();
}

void Settings::checked(int code, QString message)
{
    QMessageBox::information(0, "LogRadio.ru", "Код ответа: " + QString::number(code) +
                             "\n"+message, QMessageBox::Ok);
}

void Settings::received(QString access_token, QString confirmation_key, QString confirmation_after, QString confirmation_before, QString valid_after, QString valid_before)
{
    QMessageBox::information(0, "LogRadio.ru",
                                "Введите ключ подтверждения в разделе Токены API на LogRadio.ru\nКлюч подтверждения: " + confirmation_key + "\n" +
                                "Интервал подтверждения: от " + confirmation_after + " до " + confirmation_before + "\n" +
                                "Срок действия токена: от " + valid_after + " до " + valid_before, QMessageBox::Ok);
    ui->LogRadioAccessToken->setText(access_token);

}



