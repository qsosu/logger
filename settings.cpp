#include "settings.h"
#include "ui_settings.h"
#include <QDebug>


Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Настройки программы"));

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
    connect(logradio, &APILogRadio::errorGetToken, this, [=]() {
        QMessageBox::critical(0, tr("LogRadio.ru"), tr("Не удалось получить токен."), QMessageBox::Ok);
        return;
    });

    connect(ui->saveButton, &QPushButton::clicked, this, &Settings::save);
    connect(ui->closeButton, &QPushButton::clicked, this, &Settings::hide);
    connect(ui->getLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::getLogRadioToken);
    connect(ui->checkLogRadioTokenBtn, &QPushButton::clicked, this, &Settings::checkLogRadioToken);
    openPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/settings.ini");

    QRegularExpression ipRegex(R"((^|(0|[1-9]\d{0,2})\.(0|[1-9]\d{0,2})\.(0|[1-9]\d{0,2})\.(0|[1-9]\d{0,2})))");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(ipRegex, this);
    ui->proxyIPAddresslineEdit->setValidator(validator);

    auto re = QRegularExpression(
        R"(^(?i)[A-R]{0,2}(?:\d{0,2}(?:[A-X]{0,2}(?:\d{0,2}(?:[A-X]{0,2})?)?)?)$)",
        QRegularExpression::CaseInsensitiveOption
    );
    ui->LocatorEdit->setValidator(new QRegularExpressionValidator(re, ui->LocatorEdit));
    ui->LocatorEdit->setMaxLength(10);

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
    QString str, pwd;
    qs->beginGroup("API");
    str = qs->value("token", "").toString();
    useCallbook = qs->value("callbook", false).toBool();
    accessToken = EncryptToken(str);
    qs->endGroup();
    qs->beginGroup("APILOGRADIORU");
    str = qs->value("token", "").toString();
    logRadioAccessToken = EncryptToken(str);
    logradio->APILogRadioAccessToken = logRadioAccessToken;
<<<<<<< HEAD
    qs->endGroup();
    qs->beginGroup("PROXY");
    proxyEnable = qs->value("enable", false).toBool();
    proxyType = qs->value("proxy_type", 0).toInt();
    proxyUserName = qs->value("user", "").toString();
    pwd = qs->value("password", "").toString();
    proxyUserPassword = EncryptToken(pwd);
    proxyHost = QHostAddress(qs->value("proxy_host", "127.0.0.1").toString());
    proxyHTTPSPort = qs->value("proxy_port", 0).toInt();
=======
>>>>>>> 0970747629afdc850f928d686202adb08cbd4a29
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
    showMap = qs->value("showmap", false).toBool();
    language = qs->value("language", "").toString();
    table_row_state = qs->value("table_state", 532653568).toUInt();
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
    qs->beginGroup("LOCATION");
    Latitude = qs->value("latitude", 0.0000).toDouble();
    Longitude = qs->value("longitude", 0.0000).toDouble();
    qs->endGroup();
    display();
    ui->saveButton->setEnabled(true);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::display() {
    ui->accessToken->setText(accessToken);
    ui->LogRadioAccessToken->setText(logRadioAccessToken);
    ui->useProxyCheckBox->setChecked(proxyEnable);
    ui->proxyIPAddresslineEdit->setText(proxyHost.toString());
    ui->proxyPortlineEdit->setText(QString::number(proxyHTTPSPort));
    ui->userNameLineEdit->setText(proxyUserName);
    ui->passwordLineEdit->setText(proxyUserPassword);
    ui->proxyTypeComboBox->setCurrentIndex(proxyType);
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
    ui->languageComboBox->setCurrentText(language);
    ui->darkTheimeCheckBox->setChecked(darkTheime);
    ui->MapCheckBox->setChecked(showMap);
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
    ui->qth_lat_lineEdit->setText(QString::number(Latitude, 'f', 4));
    ui->qth_lng_lineEdit->setText(QString::number(Longitude, 'f', 4));
    ui->vt_id->setChecked(testbit(table_row_state, 0));
    ui->vt_station_callsign->setChecked(testbit(table_row_state, 4));
    ui->vt_operator->setChecked(testbit(table_row_state, 5));
    ui->vt_my_gridsquare->setChecked(testbit(table_row_state, 6));
    ui->vt_my_cnty->setChecked(testbit(table_row_state, 7));
    ui->vt_qso_date->setChecked(testbit(table_row_state, 9));
    ui->vt_time_on->setChecked(testbit(table_row_state, 10));
    ui->vt_time_off->setChecked(testbit(table_row_state, 11));
    ui->vt_freq->setChecked(testbit(table_row_state, 13));
    ui->vt_rst_send->setChecked(testbit(table_row_state, 15));
    ui->vt_rst_rcvd->setChecked(testbit(table_row_state, 16));
    ui->vt_name->setChecked(testbit(table_row_state, 17));
    ui->vt_qth->setChecked(testbit(table_row_state, 18));
    ui->vt_gridsquare->setChecked(testbit(table_row_state, 19));
    ui->vt_cnty->setChecked(testbit(table_row_state, 20));
    ui->vt_ituz->setChecked(testbit(table_row_state, 23));
    ui->vt_cqz->setChecked(testbit(table_row_state, 24));
    ui->vt_country->setChecked(testbit(table_row_state, 26));
    ui->vt_cont->setChecked(testbit(table_row_state, 27));
    ui->vt_comment->setChecked(testbit(table_row_state, 28));
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
    stream << "[PROXY]" << Qt::endl;
    stream << "enable = 0" << Qt::endl;
    stream << "proxy_type = 0" << Qt::endl;
    stream << "user = " << Qt::endl;
    stream << "password = " << Qt::endl;
    stream << "proxy_host = " << Qt::endl;
    stream << "proxy_port = " << Qt::endl;
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
    stream << "fontsize = 9" << Qt::endl;
    stream << "darktheime = true" << Qt::endl;
    stream << "language = Русский" << Qt::endl;
    stream << "showmap = false" << Qt::endl;
    stream << "table_state = 532653568" << Qt::endl;
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
    stream << Qt::endl;
    stream << "[LOCATION]" << Qt::endl;
    stream << "latitude = 0.0000" << Qt::endl;
    stream << "longitude = 0.0000" << Qt::endl;
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
    qs->beginGroup("PROXY");
    qs->setValue("enable", ui->useProxyCheckBox->isChecked() ? 1 : 0);
    qs->setValue("proxy_type", ui->proxyTypeComboBox->currentIndex());
    qs->setValue("user", ui->userNameLineEdit->text());
    qs->setValue("password", EncryptToken(ui->passwordLineEdit->text()));
    qs->setValue("proxy_host", ui->proxyIPAddresslineEdit->text());
    qs->setValue("proxy_port", ui->proxyPortlineEdit->text());
    qs->endGroup();
    qs->beginGroup("UDP");
    qs->setValue("enable", ui->udpServerEnableCheckbox->isChecked() ? 1 : 0);
    qs->setValue("port", ui->udpServerPort->value());
    qs->setValue("enable_retransl", ui->udpClientEnableCheckbox->isChecked() ? 1 : 0);
    qs->setValue("port_retransl", ui->udpClientPort->value());
    qs->endGroup();
    qs->beginGroup("CAT");
    qs->setValue("enable", ui->EnableCATcheckBox->isChecked() ? 1 : 0);
    qs->setValue("interval", ui->IntervalSpinBox->value());
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
    qs->setValue("language", ui->languageComboBox->currentText());
    qs->setValue("darktheime", ui->darkTheimeCheckBox->isChecked() ? 1 : 0);
    qs->setValue("showmap", ui->MapCheckBox->isChecked() ? 1 : 0);
    qs->setValue("table_state", saveTableState());
    qs->endGroup();
    qs->beginGroup("LOCATION");
    qs->setValue("latitude", ui->qth_lat_lineEdit->text());
    qs->setValue("longitude", ui->qth_lng_lineEdit->text());
    qs->endGroup();
    qs->sync();
    ui->saveButton->setEnabled(false);
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
   ui->registered_label->setText(tr("Дата регистрации: ") + registered);
   ui->last_activit_label->setText(tr("Последняя активность: ") + last_activity);
   premium = data.at(3);

   if(premium == "1") {
       ui->userStatusLabel->setText(tr("Подписка Premium: Активна до ") + premium_time);
       ui->CallbookCheckBox->setEnabled(true);
   }
   else ui->userStatusLabel->setText(tr("Подписка Premium: Не активна"));
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
<<<<<<< HEAD
        QMessageBox::information(0, tr("LogRadio.ru"),
                                    tr("Введите ключ подтверждения в разделе Токены API на LogRadio.ru\nКлюч подтверждения: ") + confirmation_key + "\n" +
                                    tr("Интервал подтверждения: от ") + confirmation_after + tr(" до ") + confirmation_before + "\n" +
                                    tr("Срок действия токена: от ") + valid_after + tr(" до ") + valid_before, QMessageBox::Ok);
        ui->LogRadioAccessToken->setText(access_token);
        logradio->APILogRadioAccessToken = access_token;
=======
    QMessageBox::information(0, "LogRadio.ru",
                                "Введите ключ подтверждения в разделе Токены API на LogRadio.ru\nКлюч подтверждения: " + confirmation_key + "\n" +
                                "Интервал подтверждения: от " + confirmation_after + " до " + confirmation_before + "\n" +
                                "Срок действия токена: от " + valid_after + " до " + valid_before, QMessageBox::Ok);
    ui->LogRadioAccessToken->setText(access_token);
    logradio->APILogRadioAccessToken = access_token;
>>>>>>> 0970747629afdc850f928d686202adb08cbd4a29
    } else {
        QMessageBox::critical(0, tr("LogRadio.ru"), tr("Не удалось получить токен."), QMessageBox::Ok);
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

void Settings::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::on_languageComboBox_currentIndexChanged(int index)
{
    if(index == 0) {
        qtLanguageTranslator.load("://translations/ru_Ru.qm");
        qApp->installTranslator(&qtLanguageTranslator);
    }
    if(index == 1) {
        QString path = QCoreApplication::applicationDirPath() + "/translations";
        QDir dir(path);
        QStringList filters;
        filters << "en_US.qm";

        QStringList results = dir.entryList(filters, QDir::Files);
        if(!results.isEmpty()) {
            qDebug() << "Файл найден:" << dir.filePath(results.first());
        } else {
            qDebug() << "Файл не найден.";
        }

        qtLanguageTranslator.load(dir.filePath(results.first()));
        qApp->installTranslator(&qtLanguageTranslator);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

uint Settings::saveTableState()
{
    if(ui->vt_id->isChecked()) biton(table_row_state, 0);
    else bitoff(table_row_state, 0);
    if(ui->vt_station_callsign->isChecked()) biton(table_row_state, 4);
    else bitoff(table_row_state, 4);
    if(ui->vt_operator->isChecked()) biton(table_row_state, 5);
    else bitoff(table_row_state, 5);
    if(ui->vt_my_gridsquare->isChecked()) biton(table_row_state, 6);
    else bitoff(table_row_state, 6);
    if(ui->vt_my_cnty->isChecked()) biton(table_row_state, 7);
    else bitoff(table_row_state, 7);
    if(ui->vt_qso_date->isChecked()) biton(table_row_state, 9);
    else bitoff(table_row_state, 9);
    if(ui->vt_time_on->isChecked()) biton(table_row_state, 10);
    else bitoff(table_row_state, 10);
    if(ui->vt_time_off->isChecked()) biton(table_row_state, 11);
    else bitoff(table_row_state, 11);
    if(ui->vt_freq->isChecked()) biton(table_row_state, 13);
    else bitoff(table_row_state, 13);
    if(ui->vt_rst_send->isChecked()) biton(table_row_state, 15);
    else bitoff(table_row_state, 15);
    if(ui->vt_rst_rcvd->isChecked()) biton(table_row_state, 16);
    else bitoff(table_row_state, 16);
    if(ui->vt_name->isChecked()) biton(table_row_state, 17);
    else bitoff(table_row_state, 17);
    if(ui->vt_qth->isChecked()) biton(table_row_state, 18);
    else bitoff(table_row_state, 18);
    if(ui->vt_gridsquare->isChecked()) biton(table_row_state, 19);
    else bitoff(table_row_state, 19);
    if(ui->vt_cnty->isChecked()) biton(table_row_state, 20);
    else bitoff(table_row_state, 20);
    if(ui->vt_ituz->isChecked()) biton(table_row_state, 23);
    else bitoff(table_row_state, 23);
    if(ui->vt_cqz->isChecked()) biton(table_row_state, 24);
    else bitoff(table_row_state, 24);
    if(ui->vt_country->isChecked()) biton(table_row_state, 26);
    else bitoff(table_row_state, 26);
    if(ui->vt_cont->isChecked()) biton(table_row_state, 27);
    else bitoff(table_row_state, 27);
    if(ui->vt_comment->isChecked()) biton(table_row_state, 28);
    else bitoff(table_row_state, 28);
    return table_row_state;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Settings::on_locationBtn_clicked()
{
    if (ui->LocatorEdit->text().length() < 5) {
        QMessageBox::critical(0, tr("Ошибка!"), tr("Локатор не может быть меньше 5 символов."), QMessageBox::Ok);
        return;
    }
    Coordinates latlon = locatorToCoordinates(ui->LocatorEdit->text());
    ui->qth_lat_lineEdit->setText(QString::number(latlon.latitude));
    ui->qth_lng_lineEdit->setText(QString::number(latlon.longitude));
}
//------------------------------------------------------------------------------------------------------------------------------------------

// Функция для преобразования Maidenhead Locator в географические координаты
Coordinates Settings::locatorToCoordinates(const QString& locator)
{
    QString l = locator.toUpper();
    int len = l.length();
    if (len < 2 || len % 2 != 0 || len > 12)
        return {0.0, 0.0};

    double lon = -180.0;
    double lat = -90.0;

    int pairs = len / 2;

    for (int i = 0; i < pairs; ++i) {
        QChar lonChar = l[i * 2];
        QChar latChar = l[i * 2 + 1];

        int lonVal = 0;
        int latVal = 0;
        double lonStep = 0.0;
        double latStep = 0.0;

        switch (i) {
        case 0: // Field (A-R)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 20.0;
            latStep = 10.0;
            break;
        case 1: // Square (0-9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0;
            latStep = 1.0;
            break;
        case 2: // Subsquare (A-X)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 2.0 / 24.0;    // 0.083333...
            latStep = 1.0 / 24.0;    // 0.041666...
            break;
        case 3: // Extended square (0–9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0 / 240.0;   // 0.008333...
            latStep = 1.0 / 240.0;   // 0.004166...
            break;
        case 4: // Extended subsquare (A–X)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 2.0 / 5760.0;  // 0.0003472...
            latStep = 1.0 / 5760.0;  // 0.0001736...
            break;
        case 5: // Microsquare (0–9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0 / 57600.0; // 0.00003472...
            latStep = 1.0 / 57600.0; // 0.00001736...
            break;
        default:
            break;
        }
        lon += lonVal * lonStep;
        lat += latVal * latStep;

        // На последнем уровне — добавляем половину размера ячейки, чтобы попасть в центр
        if (i == pairs - 1) {
            lon += lonStep / 2.0;
            lat += latStep / 2.0;
        }
    }
    return {lat, lon};
}
//------------------------------------------------------------------------------------------------------------------------------------------





