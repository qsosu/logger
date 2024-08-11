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
    ui->qrzruPassword->setEchoMode(QLineEdit::Password);

    connect(ui->saveButton, &QPushButton::clicked, this, &Settings::save);
    connect(ui->closeButton, &QPushButton::clicked, this, &Settings::hide);

    openPath(QCoreApplication::applicationDirPath() + "/settings.ini");
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
    qs->beginGroup("API");
    accessToken = qs->value("token", "").toString();
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
    qs->endGroup();

    display();
}

void Settings::display() {
    ui->accessToken->setText(accessToken);
    ui->udpServerEnableCheckbox->setChecked(udpServerEnable);
    ui->udpServerPort->setValue(udpServerPort);
    ui->flrigHost->setText(flrigHost.toString());
    ui->flrigPort->setValue(flrigPort);
    ui->flrigPeriod->setValue(flrigPeriod);
    ui->qrzruEnable->setChecked(enableQrzruCallbook);
    ui->qrzruLogin->setText(QrzruLogin);
    ui->qrzruPassword->setText(QrzruPassword);
    ui->fontSize->setValue(fontSize);
}

void Settings::createDefaultFile() {
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

    newFile.close();
}

void Settings::save() {
    qs->beginGroup("API");
    qs->setValue("token", ui->accessToken->text());
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
    qs->endGroup();

    qs->sync();
    emit SettingsChanged();
}
