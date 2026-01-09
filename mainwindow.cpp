#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QRegularExpressionValidator"
#include "qlistview.h"
#include <QCompleter>
#include <QSqlError>
#include <QGraphicsDropShadowEffect>
#include <QEventLoop>
#include <QDockWidget>

#define SET_ROBOT_FONT


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("QSO Logger v") + QString::fromStdString(VERSION));
    QFontDatabase::addApplicationFont("://resources/fonts/Roboto-Regular.ttf");

    //Проверка использования и версий SSL
    qInfo() << "Support SSL: " << QSslSocket::supportsSsl() << " SSL Build Library: " << QSslSocket::sslLibraryBuildVersionString() << " SSL Library Version: " << QSslSocket::sslLibraryVersionString();

//------------------------------------------------------------------------------------------------------------------------------------------
    settings = new Settings();
    settings->setAttribute(Qt::WA_QuitOnClose, false);
    connect(settings, SIGNAL(SettingsChanged()), this, SLOT(onSettingsChanged()));
    connect(ui->actionSettings, &QAction::triggered, this, [=]() {
        if(api->serviceAvailable && settings->accessToken.length() != 0) api->getUser();
        settings->show();
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    api = new HttpApi(db, settings->accessToken);
    connect(api, &HttpApi::emptyToken, this, [=]() {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не указан ACCESS TOKEN"), QMessageBox::Ok);
        return;
    });
    connect(api, &HttpApi::error, this, [=](QNetworkReply::NetworkError error) {
        QMessageBox::critical(0, tr("Ошибка"), tr("Нет связи с сервером: ") + QString::number(error), QMessageBox::Ok);
        return;
    });
    connect(api, &HttpApi::synced, this, &MainWindow::onQSOSUSynced);
    connect(api, SIGNAL(getUserInfo(QStringList)), settings, SLOT(getUserInfo(QStringList)));

    if(settings->proxyEnable) api->configureProxy(settings->proxyType, settings->proxyHost.toString(), settings->proxyHTTPSPort, settings->proxyUserName, settings->proxyUserPassword);

//------------------------------------------------------------------------------------------------------------------------------------------
    //Локальный Callbook
    localCallbook = new LocalCallbook();
//------------------------------------------------------------------------------------------------------------------------------------------
    CAT = new cat_Interface(settings->catEnable);

    qsoPanel = new QSOPanel(this, settings, CAT);
    ui->verticalLayout->insertWidget(0, qsoPanel);
    qsoPanel->dock(); // при запуске сразу в док
    ui->actionSync->setEnabled(false);

    qApp->setStyle("Fusion");
    qApp->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    setLanguage();
//------------------------------------------------------------------------------------------------------------------------------------------

    //Загрузка XML-файла с диапазонами и модуляциями
    if (LoadHamDefsSync()) {
        qInfo() << "HamDefs.xml прочитан.";
    } else {
        QMessageBox::critical(0, tr("Ошибка"), tr("Файл HamDefs.xml не найден. Завершаю работу."), QMessageBox::Ok);
        qDebug() << "Не удалось загрузить или открыть HamDefs.xml";
        close();
    }

 //------------------------------------------------------------------------------------------------------------------------------------------

    qInfo() << "Инициализация БД.";
    InitDatabase("data.db");

    //Настройка интерфейса
    ui->prevQSOtableView->setVisible(false);
    qsoPanel->setFrequenceText(settings->lastFrequence);

    // Кнопка-колокольчик
    bellBtn = new QPushButton(this);
    bellBtn->setIcon(QIcon(":/resources/images/bell.png"));
    bellBtn->setIconSize(QSize(16, 16));
    bellBtn->setFlat(true);
    bellBtn->setToolTip(tr("Нет новых уведомлений"));

    connect(bellBtn, &QPushButton::clicked, this, &MainWindow::openNewsWindow);
    statusBar()->insertPermanentWidget(0, bellBtn, 0);

    // Кнопка-конвертик
    notificationBtn = new QPushButton(this);
    notificationBtn->setIcon(QIcon(":/resources/images/notification.png"));
    notificationBtn->setIconSize(QSize(16, 16));
    notificationBtn->setFlat(true);
    notificationBtn->setToolTip(tr("Есть новые сообщения в чате"));

    connect(notificationBtn, &QPushButton::clicked, this, &MainWindow::on_actionChats_triggered);
    statusBar()->insertPermanentWidget(1, notificationBtn, 0);

    auto createSeparator = [this]() {
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setLineWidth(1);
        line->setMidLineWidth(0);
        line->setFixedWidth(1);
        line->setMaximumHeight(16);
        return line;
    };

    countQSO = new QLabel(this);
    statusBar()->addPermanentWidget(countQSO, 1);
    statusBar()->addPermanentWidget(createSeparator());
    magStormLabel = new QLabel(this);
    statusBar()->addPermanentWidget(magStormLabel);
    statusBar()->addPermanentWidget(createSeparator());
    qsosuLbl = new QLabel(this);
    statusBar()->addPermanentWidget(qsosuLbl);
    qsosuLabel = new QLabel(this);
    statusBar()->addPermanentWidget(qsosuLabel);
    statusBar()->addPermanentWidget(createSeparator());
    udpserverLbl = new QLabel(this);
    statusBar()->addPermanentWidget(udpserverLbl);
    udpserverLabel = new QLabel(this);
    statusBar()->addPermanentWidget(udpserverLabel);
    statusBar()->addPermanentWidget(createSeparator());
    flrigLbl = new QLabel(this);
    statusBar()->addPermanentWidget(flrigLbl);
    flrigLabel = new QLabel(this);
    statusBar()->addPermanentWidget(flrigLabel);
    statusBar()->addPermanentWidget(createSeparator());
    catLbl = new QLabel(this);
    statusBar()->addPermanentWidget(catLbl);
    catLabel = new QLabel(this);
    statusBar()->addPermanentWidget(catLabel);

    magStormLabel->setText(tr("Уровень МБ: "));
    udpserverLbl->setText("UDP: ");
    flrigLbl->setText("FLRIG: ");
    qsosuLbl->setText(tr("Сервис QSO.SU: "));
    qsosuLabel->setText("Offline");
    qsosuLabel->setStyleSheet("QLabel { color: red }");
    catLbl->setText("CAT: ");
    magStormLabel->setText(tr("Уровень МБ: Нет данных"));

//------------------------------------------------------------------------------------------------------------------------------------------

    udpServer = new UdpServer(this);
        if (settings->udpServerEnable) {
            if (udpServer->start(settings->udpServerPort)) {
                connect(udpServer, &UdpServer::heartbeat, this, [=]() {
                    if(!udpServer->dx_call.isEmpty()) {
                        //qInfo() << QString("UDP: Received HEARTBEAT - %1 %2").arg(QString::fromUtf8(udpServer->version), QString::fromUtf8(udpServer->revision));
                    }
                });

                connect(udpServer, &UdpServer::status, this, [=]() {
                    if(!udpServer->dx_call.isEmpty()) {
                        const QString call = QString::fromUtf8(udpServer->dx_call).trimmed();
                        // если пришёл тот же позывной — игнорируем
                        if (call.compare(lastDxCall, Qt::CaseInsensitive) == 0) return;
                        lastDxCall = call;

                        qInfo() << tr("UDP: Received STATUS") << "dx_call:" << udpServer->dx_call;
                        qsoPanel->ClearCallbookFields();
                        qsoPanel->setCallsign(QString::fromUtf8(udpServer->dx_call));
                        FindCallData(QString::fromUtf8(udpServer->dx_call));
                    }
                });

                connect(udpServer, &UdpServer::logged, this, &MainWindow::onUdpLogged);
                connect(udpServer, &UdpServer::loggedADIF, this, &MainWindow::onUdpLoggedADIF);
                ui->statusbar->showMessage(tr("UDP сервер запущен на порту ") + QString::number(settings->udpServerPort), 3000);
                udpserverLabel->setText(tr("Запущен"));
                udpserverLabel->setStyleSheet("QLabel { color: green }");
            } else {
                udpserverLabel->setText(tr("Ошибка"));
                udpserverLabel->setStyleSheet("QLabel { color: red }");
            }
        }
        if (settings->udpClientEnable) {
            udpServer->setRetransl(true);
            udpServer->setRetranslPort(settings->udpClientPort);
        }
//------------------------------------------------------------------------------------------------------------------------------------------

    flrig = new Flrig(settings->flrigHost, settings->flrigPort, 500, this);
    flrigLabel->setText(tr("Отключен"));
    flrigLabel->setStyleSheet("QLabel { color: red; }");
    connect(flrig, &Flrig::connected, this, [=]() {
        flrigLabel->setText(tr("Подключен"));
        flrigLabel->setStyleSheet("QLabel { color: green; }");
        ui->statusbar->showMessage(tr("Подключен к FLRIG"), 1000);
    });
    connect(flrig, &Flrig::disconnected, this, [=]() {
        flrigLabel->setText(tr("Отключен"));
        flrigLabel->setStyleSheet("QLabel { color: red; }");
        ui->statusbar->showMessage(tr("Отключен от FLRIG"), 1000);
    });
    connect(flrig, &Flrig::updated, this, [=]() {
        if (flrig->getConnState()) {
            qsoPanel->setBand(Helpers::GetBandByFreqHz(flrig->getFrequencyHz()));
            qsoPanel->setFrequence((double) flrig->getFrequencyHz());
            qsoPanel->setMode(flrig->getMode());
        }
    });
    connect(flrig, &Flrig::rpcError, this, [=]() {
        ui->statusbar->showMessage(QString(tr("Ошибка XML-RPC: %1 %2")).arg(QString::number(flrig->getErrorCode()), flrig->getErrorString()), 300);
        qDebug() << QString(tr("Ошибка XML-RPC: %1 %2")).arg(QString::number(flrig->getErrorCode()), flrig->getErrorString());
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    connect(ui->actionCAT, &QAction::toggled, this, [=](bool state) {
        if(state) {
            CAT->catTimer->start();
            settings->catEnable = true;
            catLabel->setText(tr("Включен"));
            catLabel->setStyleSheet("QLabel { color: green; }");
        } else {
            CAT->catTimer->stop();
            settings->catEnable = false;
            catLabel->setText(tr("Отключен"));
            catLabel->setStyleSheet("QLabel { color: red; }");
        }
    });

    if(settings->catEnable) {
        bool portAvailable = CAT->openSerial(settings->serialPort);
        if(portAvailable) {
            CAT->setInterval(settings->catInterval);
            CAT->catSetBaudRate(settings->serialPortBaud.toInt());
            CAT->catSetDataBits(settings->serialPortDataBits.toInt());
            CAT->catSetStopBit(settings->serialPortStopBit.toInt());
            CAT->catSetParity(settings->serialPortParity);
            CAT->catSetFlowControl(settings->serialPortFlowControl);
            connect(CAT, SIGNAL(cat_freq(long)), this, SLOT(setFreq(long)));
            connect(CAT, SIGNAL(cat_band(int)), this, SLOT(setBand(int)));
            connect(CAT, SIGNAL(cat_mode(int)), this, SLOT(setMode(int)));
        }
    }

//------------------------------------------------------------------------------------------------------------------------------------------
    qrz = new QrzruCallbook(settings->QrzruLogin, settings->QrzruPassword);
    connect(qrz, &QrzruCallbook::error404, this, [=]() {
        ui->statusbar->showMessage(tr("QRZ API - данные не найдены"), 2000);
    });
    connect(qrz, &QrzruCallbook::error, this, [=]() {
        ui->statusbar->showMessage(tr("QRZ API - ошибка запроса"), 2000);
    });
    connect(qsoPanel, &QSOPanel::findCallTimer, this, [=]() {
        if (settings->enableQrzruCallbook || settings->useCallbook || settings->useLocalCallbook)
            FindCallData(qsoPanel->getCallsign());
    });

//------------------------------------------------------------------------------------------------------------------------------------------

    QsoSuPingTimer = new QTimer(this);
    QsoSuPingTimer->setInterval(5000); //5 секунд
    connect(QsoSuPingTimer, &QTimer::timeout, this, [=]() {
        PingQsoSu();
    });
    QsoSuPingTimer->start();
    PingQsoSu();

//------------------------------------------------------------------------------------------------------------------------------------------

    QrzRuPingTimer = new QTimer(this);
    QrzRuPingTimer->setInterval(5000); //5 секунд
    connect(QrzRuPingTimer, &QTimer::timeout, this, [=]() {
        PingQrzRu();
    });
    QrzRuPingTimer->start();
    PingQrzRu();

//------------------------------------------------------------------------------------------------------------------------------------------

    callsigns = new Callsigns(db, api, this);
    callsigns->setAttribute(Qt::WA_QuitOnClose, false);
    connect(callsigns, &Callsigns::updated, this, &MainWindow::onCallsignsUpdated);
    connect(ui->actionCallsigns, &QAction::triggered, this, [=]() {
        callsigns->show();
    });
    connect(api, &HttpApi::callsignsUpdated, callsigns, &Callsigns::onCallsignsUpdated);

    connect(ui->actionFlrig, &QAction::toggled, this, [=](bool state) {
        if (state) {
            flrig->start();
        } else {
            flrig->stop();
        }
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    connect(qsoPanel, SIGNAL(stationCallsignIndexChanged(int)), this, SLOT(onStationCallsignChanged()));
    connect(qsoPanel, SIGNAL(operatorCallsignIndexChanged(int)), this, SLOT(onOperatorChanged()));
    connect(qsoPanel, SIGNAL(modeTextChanged(QString)), this, SLOT(onModeChanged(QString)));
    connect(qsoPanel, SIGNAL(bandIndexChanged(int)), this, SLOT(fillDefaultFreq()));
    connect(qsoPanel, SIGNAL(saveQSO()), this, SLOT(SaveQso()));
    connect(qsoPanel, &QSOPanel::updateDB, this, [=] {
        RefreshRecords();
        ScrollRecordsToTop();
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    osm = new Geolocation();

    Coordinates latlon;
    latlon = osm->locatorToCoordinates(userData.gridsquare);
    settings->Latitude = latlon.latitude;
    settings->Longitude = latlon.latitude;
    osm->qth_lat = latlon.latitude;
    osm->qth_lng = latlon.longitude;

    gc = new GlobeContainer(":/resources/images/earth.jpg");
//------------------------------------------------------------------------------------------------------------------------------------------

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClickedQSO(QModelIndex)));
    connect(ui->prevQSOtableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClickedPrevQSO(QModelIndex)));
    connect(ui->prevQSOtableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(prevMenuRequested(QPoint)));

//------------------------------------------------------------------------------------------------------------------------------------------

    logradio = new APILogRadio(settings->logRadioAccessToken);
    connect(logradio, &APILogRadio::synced, this, &MainWindow::onLogRadioSynced);
    connect(logradio, &APILogRadio::errorGetToken, this, [=]() {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не удалось получить токен!"), QMessageBox::Ok);
        return;
    });
//------------------------------------------------------------------------------------------------------------------------------------------
    connect(ui->actionAbout, &QAction::triggered, this, [=]() {
        about = new About(this);
        about->exec();
        return;
    });
    connect(ui->actionQsosuLink, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://qso.su/"));
    });
    connect(ui->actionQsosuFaqLink, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://qso.su/ru/faq"));
    });
//------------------------------------------------------------------------------------------------------------------------------------------
    connect(api, SIGNAL(userDataUpdated(bool)), this, SLOT(onQsoSuResult(bool)));

    InitRecordsTable();
    InitPreviosQSOModel();

//------------------------------------------------------------------------------------------------------------------------------------------
    getCallsigns();
    fillDefaultFreq();

    if (settings->darkTheime) darkTheme();
    else lightTheme();

    catLabel->setText(tr("Отключен"));
    catLabel->setStyleSheet("QLabel { color: red; }");

//------------------------------------------------------------------------------------------------------------------------------------------

    qInfo() << tr("Загрузка файлов с префиксами.");
    ctyData = loadCtyDatabase("cty.dat"); //Загружаем файл с префиксами ARRL
    if(ctyData.count() > 0) qInfo() << tr("Файл с префиксами cty.dat загружен.");
    else qInfo() << tr("Не удалось загрузить файл с префиксами.");
    update_prefix = new UpdateLogPrefix(db, ctyData);
    connect(update_prefix, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));

//------------------------------------------------------------------------------------------------------------------------------------------

    reportSunChart = new ReportSunChart(db);
    connect(api, SIGNAL(MagStormUpdated(QJsonArray)), reportSunChart, SLOT(InsertMeasurement(QJsonArray)));
    connect(api, SIGNAL(MagStormCurrentUpdated(QJsonObject)), this, SLOT(UpdateMeasurement(QJsonObject)));

//------------------------------------------------------------------------------------------------------------------------------------------
    if(settings->showMap) on_actionShowMap_triggered();
    loadCallList();
//------------------------------------------------------------------------------------------------------------------------------------------
    MagStormTimer = new QTimer(this);
    MagStormTimer->setInterval(10800000); //3 часа
    connect(MagStormTimer, &QTimer::timeout, this, [=]() {
        MagStormUpdate();
    });
    MagStormTimer->start();
//------------------------------------------------------------------------------------------------------------------------------------------
    //Восстанавливаем последнее состояние контролов
    qsoPanel->setBand(settings->lastBand);
    if(settings->lastMode == "") qsoPanel->setMode("CW");
        else qsoPanel->setMode(settings->lastMode);

    qsoPanel->setQTHLocator(settings->lastLocator);
    qsoPanel->setRDA(settings->lastRDA);
    qsoPanel->setFrequenceText(settings->lastFrequence);
    qsoPanel->setRSTR(settings->lastRST_RCVD);
    qsoPanel->setRSTS(settings->lastRST_SENT);
    qsoPanel->setStationCallsignCurrentIndex(settings->lastCallsign);
    qsoPanel->setStationOperatorCurrentIndex(settings->lastOperator);
//------------------------------------------------------------------------------------------------------------------------------------------
    qsoedit = new Qsoedit(db, bandList, modeList, settings);
    connect(qsoedit, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));
//------------------------------------------------------------------------------------------------------------------------------------------

    api->getListSpotServers();
    connect(api, &HttpApi::spotServersReceived, this, [=](const QList<ServerInfo> &servers){
        if (servers.isEmpty()) {
            qWarning() << tr("Список серверов пуст!");
            return;
        }
        qInfo() << tr("Доступные серверы:");
        // Ищем сервер с минимальным client
        const ServerInfo *best = &servers.first();
        for (const auto &s : servers) {
            if (s.client < best->client) {
                best = &s;
            }
            qInfo() << best->host << best->port << "client =" << best->client;
        }

        tclient = new TelnetClient(db, best->host, best->port, this);

        spotViewer = new SpotViewer(db, bandList, modeList, ctyData, CAT);
        connect(spotViewer, SIGNAL(setNewQSO(QString, QString, double, QString)), this, SLOT(setSpotInfo(QString, QString, double, QString)));

        chats = new ChatController(db, api);
        connect(tclient, SIGNAL(newsMessageReceived()), this, SLOT(showBellIcon()));
        connect(tclient, SIGNAL(newChatReseived()), this, SLOT(showNotificationIcon()));
        connect(tclient, &TelnetClient::chatMessageReceived, chats, &ChatController::onMessageReceived);
        connect(tclient, SIGNAL(newSpotReseived()), spotViewer, SLOT(updateSpots()));
        qInfo() << tr("Подключаемся к серверу:") << best->host << best->port << "client =" << best->client;
        spotServerConnected = true;
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    connect(api, &HttpApi::serviceAvailableChanged, this, [this](bool available)
    {
        if (available && !serviceWasAvailable) {
            serviceWasAvailable = true;
            RemoveDeferredQSOs(); //Удаляем с QSO.SU ранее не удаленные QSO
            api->getMagneticStormCurrent();
        }
    });

    qInfo() << "QSOLogger v" << VERSION << " started.";
    qInfo() << "Product Name: " << QSysInfo::prettyProductName();
    RefreshRecords();
}
//------------------------------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
  delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    tclient->disconnectFromServer();
    if (osm->isVisible()) osm->close();
    if (spotViewer->isVisible()) spotViewer->close();
    if (chats->isVisible()) chats->close();
    if (reportSunChart->isVisible()) reportSunChart->close();
    if (qsoPanel->isVisible()) qsoPanel->close();
    if (gc->globe->isVisible()) gc->globe->close();
    if (gc->isVisible()) gc->close();

    saveHeaderState(ui->tableView);
    QMainWindow::closeEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::resizeEvent(QResizeEvent *event)
{

    QMainWindow::resizeEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ReinitSettings()
{
    settings->read();
    settings->display();

    if (settings->darkTheime) {
        darkTheme();
        darkTheme();
    } else {
        lightTheme();
        lightTheme();
    }

    api->setAccessToken(settings->accessToken); //Обновляем AccessToken
    if(settings->proxyEnable) api->configureProxy(settings->proxyType, settings->proxyHost.toString(), settings->proxyHTTPSPort, settings->proxyUserName, settings->proxyUserPassword);

    if(settings->catEnable) {
        if(COMPortAvailable) {
            CAT->closeSerial();
            COMPortAvailable = false;
        }
        COMPortAvailable = CAT->openSerial(settings->serialPort);
        if(COMPortAvailable) {
            CAT->setInterval(settings->catInterval);
            CAT->catSetBaudRate(settings->serialPortBaud.toInt());
            CAT->catSetDataBits(settings->serialPortDataBits.toInt());
            CAT->catSetStopBit(settings->serialPortStopBit.toInt());
            CAT->catSetParity(settings->serialPortParity);
            CAT->catSetFlowControl(settings->serialPortFlowControl);
            connect(CAT, SIGNAL(cat_freq(long)), this, SLOT(setFreq(long)));
            connect(CAT, SIGNAL(cat_band(int)), this, SLOT(setBand(int)));
            connect(CAT, SIGNAL(cat_mode(int)), this, SLOT(setMode(int)));
        }
    }

    udpServer->stop();
    udpServer->start(settings->udpServerPort);

    qApp->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    ui->tableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    ui->prevQSOtableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    osm->qth_lat = settings->Latitude;
    osm->qth_lng = settings->Longitude;
}
//------------------------------------------------------------------------------------------------------------------------------------------
/* Database section */

void MainWindow::InitDatabase(QString dbFile)
{
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).exists())
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    database_file = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + dbFile;
    if (!CheckDatabase()) {
        qWarning() << "Database file does not exist. Creating new.";
        CreateDatabase();
    }
    if (ConnectDatabase()) {
        ui->statusbar->showMessage(tr("Файл БД открыт"), 3000);
    } else {
        qWarning() << "Error while open database file";
        //QMessageBox::critical(0, tr("Ошибка"), tr("Ошибка открытия файла БД"), QMessageBox::Ok);
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::CheckDatabase() {
    QFileInfo check_file(database_file);
    if (check_file.exists() && check_file.isFile()) return true;
    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::CreateDatabase()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "CreateConnection");
  qInfo() << "Database path:" << database_file;
  db.setDatabaseName(database_file);
  if (!db.open()) {
    qWarning() << "Can't open new database file";
    return;
  }
  QSqlQuery query(db);
  qInfo() << "Creating CALLSIGNS table";
  query.exec("PRAGMA foreign_keys = ON");
  query.exec("CREATE TABLE \"callsigns\" (\"id\" INTEGER NOT NULL, \"qsosu_id\" INTEGER NOT NULL DEFAULT 0, \"type\" INTEGER NOT NULL DEFAULT 10, \"name\" TEXT NOT NULL, \"validity_start\" INTEGER NOT NULL DEFAULT 0, \"validity_stop\" INTEGER NOT NULL DEFAULT 0, \"gridsquare\" TEXT NOT NULL, \"cnty\" TEXT, \"ituz\" INTEGER NOT NULL DEFAULT 0, \"cqz\" INTEGER NOT NULL DEFAULT 0, \"status\" TEXT, PRIMARY KEY(\"id\" AUTOINCREMENT))");

  qInfo() << "Creating RECORDS table";
  query.exec("CREATE TABLE \"records\" ("
             "\"id\" INTEGER NOT NULL,"
             "\"callsign_id\" INTEGER NOT NULL,"
             "\"qsosu_callsign_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"qsosu_operator_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"STATION_CALLSIGN\" TEXT NOT NULL,"
             "\"OPERATOR\" TEXT,"
             "\"MY_GRIDSQUARE\" TEXT,"
             "\"MY_CNTY\" TEXT,"
             "\"CALL\" TEXT NOT NULL,"
             "\"QSO_DATE\" TEXT,"
             "\"TIME_ON\" TEXT,"
             "\"TIME_OFF\" TEXT,"
             "\"BAND\" TEXT,"
             "\"FREQ\" INTEGER,"
             "\"MODE\" TEXT,"
             "\"RST_SENT\" TEXT,"
             "\"RST_RCVD\" TEXT,"
             "\"NAME\" TEXT,"
             "\"QTH\" TEXT,"
             "\"GRIDSQUARE\" TEXT,"
             "\"CNTY\" TEXT,"
             "\"sync_state\" INTEGER NOT NULL DEFAULT 0,"
             "\"HASH\" TEXT,"
             "\"ITUZ\" INTEGER,"
             "\"CQZ\" INTEGER,"
             "\"SYNC_QSO\" INTEGER,"
             "\"COUNTRY\" TEXT,"
             "\"CONT\" TEXT,"
             "\"COUNTRY_CODE\" TEXT,"
             "\"COMMENT\" TEXT,"
             "\"QSL_STATUS\" INTEGER DEFAULT 0,"
             "PRIMARY KEY(\"id\" AUTOINCREMENT))");
  qInfo() << "Creating UNIQUE INDEX";
  query.exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_records_hash ON RECORDS (HASH) WHERE HASH IS NOT NULL AND HASH <> ''");
  query.exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_records_unique ON RECORDS (CALL, QSO_DATE, TIME_ON, BAND, MODE);");
  qInfo() << "Creating DELRECORDS table";
  query.exec("CREATE TABLE \"delrecords\" (\"id\" INTEGER NOT NULL, \"HASH\" TEXT, PRIMARY KEY(\"id\" AUTOINCREMENT))");
  qInfo() << "Creating SYNCHRONIZATION table";
  query.exec("CREATE TABLE \"synchronization\" (\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, \"key\" INTEGER NOT NULL, "
             "\"id_log\" INTEGER NOT NULL, \"time_update\" INTEGER, FOREIGN KEY (\"id_log\") REFERENCES \"records\" (\"id\") ON DELETE CASCADE)");
  qInfo() << "Creating LAST_CONFIRM table";
  query.exec("CREATE TABLE \"last_confirm\"(\"date\" Text  NOT NULL)");
  qInfo() << "Creating TEMP_RECORDS table";
  query.exec("CREATE TABLE \"temp_records\" ("
             "\"callsign_id\" INTEGER NOT NULL,"
             "\"qsosu_callsign_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"qsosu_operator_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"STATION_CALLSIGN\" TEXT NOT NULL,"
             "\"OPERATOR\" TEXT,"
             "\"MY_GRIDSQUARE\" TEXT,"
             "\"MY_CNTY\" TEXT,"
             "\"CALL\" TEXT NOT NULL,"
             "\"QSO_DATE\" TEXT,"
             "\"TIME_ON\" TEXT,"
             "\"TIME_OFF\" TEXT,"
             "\"BAND\" TEXT,"
             "\"FREQ\" INTEGER,"
             "\"MODE\" TEXT,"
             "\"RST_SENT\" TEXT,"
             "\"RST_RCVD\" TEXT,"
             "\"NAME\" TEXT,"
             "\"QTH\" TEXT,"
             "\"GRIDSQUARE\" TEXT,"
             "\"CNTY\" TEXT,"
             "\"sync_state\" INTEGER NOT NULL DEFAULT 0,"
             "\"HASH\" TEXT,"
             "\"ITUZ\" INTEGER,"
             "\"CQZ\" INTEGER,"
             "\"SYNC_QSO\" INTEGER,"
             "\"COUNTRY\" TEXT,"
             "\"CONT\" TEXT,"
             "\"COUNTRY_CODE\" TEXT,"
             "\"COMMENT\" TEXT,"
             "\"QSL_STATUS\" INTEGER DEFAULT 0)");
  qInfo() << "Creating UNIQUE INDEX";
  query.exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_temp_records_unique ON temp_records (CALL, QSO_DATE, TIME_ON, BAND, MODE);");
  qInfo() << "Creating MAGNETIC_STORM table";
  query.exec("CREATE TABLE \"magnetic_storm\" ("
             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
             "\"MEASUREMENT_TIME\" TEXT, "
             "\"VALUE\" INTEGER )");
  qInfo() << "Creating NEWS table";
  query.exec("CREATE TABLE \"news\" ("
             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
             "\"date\" TEXT, "
             "\"title\" TEXT, "
             "\"text\" TEXT"
             ")");
  qInfo() << "Creating SPOTS table";
  query.exec("CREATE TABLE \"spots\" ("
             "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT, "
             "\"spotter\" TEXT, "
             "\"callsign\" TEXT, "
             "\"frequency\" REAL, "
             "\"message\" TEXT, "
             "\"message_raw\" TEXT, "
             "\"mode\" TEXT, "
             "\"submode\" TEXT, "
             "\"band\" TEXT, "
             "\"wave_type\" TEXT, "
             "\"event_at\" TEXT, "
             "\"dxcc_country\" TEXT, "
             "\"dxcc_country_code\" TEXT, "
             "\"dxcc_continent\" TEXT, "
             "\"dxcc_cqz\" TEXT, "
             "\"dxcc_ituz\" TEXT"
             ")");
  qInfo() << "Creating UNIQUE INDEX";
  query.exec(
      "CREATE UNIQUE INDEX IF NOT EXISTS idx_spots_unique "
      "ON spots(spotter, callsign, event_at)"
  );
  qInfo() << "Creating triggers";
  query.exec(
      "CREATE TRIGGER \"limit_spots\" "
      "AFTER INSERT ON \"spots\" "
      "BEGIN "
      "DELETE FROM \"spots\" "
      "WHERE \"id\" < ("
          "SELECT \"id\" FROM \"spots\" "
          "ORDER BY \"id\" DESC "
          "LIMIT 1 OFFSET 1000"
      "); "
      "END;");
  query.finish();
  db.close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::ConnectDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(database_file);
    if (!db.open()) return false;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            SaveQso();
        }

        if (event->key() == Qt::Key_Escape) {
            qsoPanel->clearQSO();
        }
    }
    if (event->modifiers() & Qt::ALT && event->key() == Qt::Key_S)
    {
        //qDebug() << "Alt+S нажаты одновременно!";

    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::getCallsigns()
{
    qsoPanel->clearStationCallsignItems();
    qsoPanel->clearOperatorCallsignItems();

    QSqlQuery query(db);
    query.exec("SELECT id, qsosu_id, type, name, gridsquare, cnty FROM callsigns");
    while(query.next()) {
        int id = query.value(0).toInt();
        int qsosu_id = query.value(1).toInt();
        int type = query.value(2).toInt();
        QString name = query.value(3).toString();
        QString gridsquare = query.value(4).toString();
        QString cnty = query.value(5).toString();

        qsoPanel->addStationCallsignItems(name, QList<QVariant>() << id << qsosu_id << type << gridsquare << cnty);
        if(type == 0) qsoPanel->addOperatorCallsignItems(name, QList<QVariant>() << id << qsosu_id);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitRecordsTable()
{
  RecordsModel = new ColorSqlTableModel(this);
  RecordsModel->setTable("records");
  RecordsModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
  RecordsModel->setHeaderData(4, Qt::Horizontal, tr("Станция"));
  RecordsModel->setHeaderData(5, Qt::Horizontal, tr("Оператор"));
  RecordsModel->setHeaderData(6, Qt::Horizontal, tr("Локатор станции"));
  RecordsModel->setHeaderData(7, Qt::Horizontal, tr("CNTY станции"));
  RecordsModel->setHeaderData(8, Qt::Horizontal, tr("Позывной"));
  RecordsModel->setHeaderData(9, Qt::Horizontal, tr("Дата"));
  RecordsModel->setHeaderData(10, Qt::Horizontal, tr("Время нач."));
  RecordsModel->setHeaderData(11, Qt::Horizontal, tr("Время оконч."));
  RecordsModel->setHeaderData(12, Qt::Horizontal, tr("Диапазон"));
  RecordsModel->setHeaderData(13, Qt::Horizontal, tr("Частота"));
  RecordsModel->setHeaderData(14, Qt::Horizontal, tr("Мода"));
  RecordsModel->setHeaderData(15, Qt::Horizontal, tr("RST отпр."));
  RecordsModel->setHeaderData(16, Qt::Horizontal, tr("RST прин."));
  RecordsModel->setHeaderData(17, Qt::Horizontal, tr("Имя"));
  RecordsModel->setHeaderData(18, Qt::Horizontal, tr("QTH"));
  RecordsModel->setHeaderData(19, Qt::Horizontal, tr("Локатор"));
  RecordsModel->setHeaderData(20, Qt::Horizontal, tr("Район"));
  RecordsModel->setHeaderData(21, Qt::Horizontal, tr("CFM"));
  RecordsModel->setHeaderData(23, Qt::Horizontal, tr("ITUZ"));
  RecordsModel->setHeaderData(24, Qt::Horizontal, tr("CQZ"));
  RecordsModel->setHeaderData(26, Qt::Horizontal, tr("Территория"));
  RecordsModel->setHeaderData(27, Qt::Horizontal, tr("Континент"));
  RecordsModel->setHeaderData(28, Qt::Horizontal, tr("Флаг"));
  RecordsModel->setHeaderData(29, Qt::Horizontal, tr("Коммент."));
  RecordsModel->setHeaderData(30, Qt::Horizontal, tr("Статус отправки"));

  if(settings->logRadioAccessToken.length() != 0) RecordsModel->services = 2;
  else RecordsModel->services = 1;
  ui->tableView->setModel(RecordsModel);
  ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  setTableRow();
  ui->tableView->setItemDelegateForColumn(8, new FormatCallsign(ui->tableView));
  ui->tableView->setItemDelegateForColumn(9, new FormatDate(ui->tableView));
  ui->tableView->setItemDelegateForColumn(10, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(11, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(13, new FormatFreq(ui->tableView));
  ui->tableView->setItemDelegateForColumn(21, new FormatSyncState(ui->tableView));
  ui->tableView->horizontalHeader()->moveSection(21, 0);
  ui->tableView->horizontalHeader()->moveSection(28, 9);
  ui->tableView->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView *horizontalHeader = ui->tableView->horizontalHeader();
  horizontalHeader->setSectionResizeMode(QHeaderView::Interactive);
  horizontalHeader->setMinimumSectionSize(35);
  horizontalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
  horizontalHeader->setStretchLastSection(true);
  horizontalHeader->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));

  QHeaderView *verticalHeader = ui->tableView->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setDefaultSectionSize(20);

  // Разрешаем перетаскивание столбцов
  ui->tableView->horizontalHeader()->setSectionsMovable(true);
  ui->tableView->horizontalHeader()->setDragEnabled(true);
  ui->tableView->horizontalHeader()->setDragDropMode(QAbstractItemView::InternalMove);

  // Восстанавливаем порядок столбцов
  restoreHeaderState(ui->tableView);

  ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->prevQSOtableView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
  ui->tableView->selectRow(0);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SetRecordsFilter(int log_id)
{
    RecordsModel->setFilter(QString("callsign_id=%1").arg(log_id));
    RecordsModel->setSort(0, Qt::DescendingOrder); // Sort by ID
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RefreshRecords()
{
    RecordsModel->select();
    while (RecordsModel->canFetchMore()) // FIX. Fetch more than 256 records!!!
        RecordsModel->fetchMore();
    ScrollRecordsToTop();
    countQSO->setText(tr("Всего QSO: ") + QString::number(RecordsModel->rowCount()));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::customMenuRequested(QPoint pos)
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if (indexes.count() == 0) return;

    QMenu *menu = new QMenu(this);

    QAction *deleteAction = new QAction((indexes.count() > 1) ? tr("Удалить выбранные") : tr("Удалить"), this);
    connect(deleteAction, &QAction::triggered, this, [=]() {
        RemoveQSOs(indexes);
    });
    QAction *syncAction = new QAction((indexes.count() > 1) ? tr("Синхронизировать выбранные") : tr("Синхронизировать"), this);
    connect(syncAction, &QAction::triggered, this, [=]() {
        SyncQSOs(indexes);
    });
    QAction *qsoEditAction = new QAction(tr("Редактировать QSO"), this);
    connect(qsoEditAction, &QAction::triggered, this, [=]() {
        EditQSO(indexes.at(0));
    });
    QAction *sendSpotAction = new QAction(tr("Отправить спот"), this);
    connect(sendSpotAction, &QAction::triggered, this, [=]() {
        SendSpotQSO(indexes.at(0));
    });
    menu->addAction(deleteAction);
    menu->addAction(syncAction);
    menu->addAction(qsoEditAction);
    menu->addAction(sendSpotAction);
    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::prevMenuRequested(QPoint pos)
{
    QModelIndexList indexes = ui->prevQSOtableView->selectionModel()->selectedRows();
    if (indexes.count() == 0) return;

    QMenu *menu = new QMenu(this);

    int row = indexes.at(0).row();
    int id = PrevRecordsModel->data(PrevRecordsModel->index(row, 0)).toInt(); // столбец 0 — id

    QModelIndex index = findIndexById(RecordsModel, id);
    if (index.isValid()) {
        QAction *qsoEditAction = new QAction(tr("Редактировать QSO"), this);
        connect(qsoEditAction, &QAction::triggered, this, [=]() {
            EditQSO(index);
        });

        QAction *deleteAction = new QAction((indexes.count() > 1) ? tr("Удалить выбранные") : tr("Удалить"), this);
        connect(deleteAction, &QAction::triggered, this, [=]() {
            RemoveQSOs(indexes);
        });
        menu->addAction(qsoEditAction);
    }
    menu->popup(ui->prevQSOtableView->viewport()->mapToGlobal(pos));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SaveQso()
{
    if(qsoPanel->getCallsign() == "") {
        QMessageBox::critical(0, tr("Ошибка"), tr("Введите позывной кореспондента!"), QMessageBox::Ok);
        return;
    }
    if(qsoPanel->getRSTR() == "" && qsoPanel->getRSTS() == "") {
        QMessageBox::critical(0, tr("Ошибка"), tr("Введите рапорта QSO!"), QMessageBox::Ok);
        return;
    }
    QSqlRecord newRecord = RecordsModel->record();
    newRecord.remove(newRecord.indexOf("id"));
    newRecord.setValue("callsign_id", userData.callsign_id);
    newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
    newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
    newRecord.setValue("STATION_CALLSIGN", userData.callsign);
    newRecord.setValue("OPERATOR", userData.oper);

    QString call = qsoPanel->getCallsign();
    newRecord.setValue("CALL", call);

    CountryEntry result = findCountryByCall(call, ctyData);
    if(!result.country.isEmpty())
    {
        newRecord.setValue("COUNTRY", result.country);
        newRecord.setValue("COUNTRY_CODE", countryToIso.value(result.country, ""));
        newRecord.setValue("CONT", result.continent);
        newRecord.setValue("ITUZ", result.ituZone);
        newRecord.setValue("CQZ", result.cqZone);
    }

    QDate qsoDate = qsoPanel->getDate();
    newRecord.setValue("QSO_DATE", qsoDate.toString("yyyyMMdd"));
    QTime qsoTime = qsoPanel->getTime();
    QString date = qsoDate.toString("yyyy-MM-dd");
    QString time = qsoTime.toString("hh:mm:ss");
    QString datetime = date + "T" + time;
    QString qsoTimeFormated = qsoTime.toString("hhmmss");
    newRecord.setValue("TIME_ON", qsoTimeFormated);
    newRecord.setValue("TIME_OFF", qsoTimeFormated);
    QString band = qsoPanel->getBand();
    newRecord.setValue("BAND", band);
    unsigned long long freqHz = static_cast<unsigned long long>(qsoPanel->getFrequence().toDouble() * 1000000);
    newRecord.setValue("FREQ", freqHz);
    QString mode = getModeValue(qsoPanel->getMode());
    newRecord.setValue("MODE", mode.toUpper());
    QString rsts = qsoPanel->getRSTS();
    newRecord.setValue("RST_SENT", rsts);
    QString rstr = qsoPanel->getRSTR();
    newRecord.setValue("RST_RCVD", rstr);
    QString name = qsoPanel->getName();
    newRecord.setValue("NAME",name);
    QString qth = qsoPanel->getQTH();
    newRecord.setValue("QTH", qth);
    QString gridsquare = qsoPanel->getQTHLocator();
    newRecord.setValue("GRIDSQUARE", gridsquare);
    QString cnty = qsoPanel->getRDA();
    newRecord.setValue("CNTY", cnty);
    newRecord.setValue("COMMENT", qsoPanel->getComment());
    newRecord.setValue("sync_state", 0);
    QString my_gridsquare = qsoPanel->getStationQTHLocator();
    newRecord.setValue("MY_GRIDSQUARE", my_gridsquare);
    QString my_cnty = qsoPanel->getStationRDA();
    newRecord.setValue("MY_CNTY", my_cnty);
    newRecord.setValue("QSL_STATUS", 0);

    if (RecordsModel->insertRecord(-1, newRecord)) {
        RecordsModel->submitAll();

        // Здесь надо отправлять QSO на сервис через API
        int LastID = RecordsModel->query().lastInsertId().toInt();
        QVariantList data;
        data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare << my_gridsquare << my_cnty;

        if(api->serviceAvailable) api->SendQso(data);
        if(qrz->serviceAvailable) {
            data << userData.callsign << userData.oper;
            logradio->SendQso(data);
        }
        RefreshRecords();
        ScrollRecordsToTop();
        qsoPanel->SaveCallsignState();
        qsoPanel->clearQSO();
    } else {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не возможно сохранить QSO. Ошибка базы данных!"), QMessageBox::Ok);
        qDebug() << "Ошибка: " << RecordsModel->lastError();
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RemoveQSOs(QModelIndexList indexes)
{
    int countRow = indexes.count();
    if (countRow == 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Подтверждение действия"), QString(tr("Вы уверены что хотите удалить %1 QSO?")).arg(countRow), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        db.transaction();
        QSqlQuery query(db);
        query.exec("PRAGMA foreign_keys = ON");
        query.prepare("DELETE FROM records WHERE id = ?");
        for (int i = countRow; i > 0; i--) {
            int id = RecordsModel->data(RecordsModel->index(indexes.at(i-1).row(), 0)).toInt();

            QString hash = RecordsModel->data(RecordsModel->index(indexes.at(i-1).row(), 23), 0).toString();
            if(api->serviceAvailable) api->deleteByHashLog(hash);
            else insertDataToDeferredQSOs(id, hash);

            query.addBindValue(id);
            query.exec();
        }
        db.commit();
        RefreshRecords();
        ScrollRecordsToTop();
    } else {
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onSettingsChanged()
{
    ReinitSettings();
    setTableRow();
    if(api->serviceAvailable && settings->accessToken.length() != 0 && !tclient->TelnetConnected) {
        api->getUser();
        api->getListSpotServers();
        QApplication::processEvents();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ScrollRecordsToBottom()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->selectRow(RecordsModel->rowCount() - 1);
    ui->tableView->scrollToBottom();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ScrollRecordsToTop()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->selectRow(0);
    ui->tableView->scrollToTop();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onCallsignsUpdated()
{
  getCallsigns();

  if (chats) {
      delete chats;
      chats = nullptr;
  }
  chats = new ChatController(db, api);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onStationCallsignChanged()
{
    auto data = qsoPanel->getStationCallsignItems(qsoPanel->getStationCallsign());
    userData.callsign_id = data.value(0).toInt();
    userData.qsosu_callsign_id = data.value(1).toInt();
    userData.callsign = qsoPanel->getStationCallsign();
    userData.gridsquare = data.value(3).toString();
    userData.cnty = data.value(4).toString();

    qsoPanel->setStationQTHLocator(userData.gridsquare);
    qsoPanel->setStationRDA(userData.cnty);

    SetRecordsFilter(userData.callsign_id);
    RefreshRecords();
    ScrollRecordsToTop();

    showPreviosQSO("");

    if(qsoPanel->getStationCallsign() != "")
    {
        Coordinates latlon = osm->locatorToCoordinates(userData.gridsquare);
        settings->Latitude = latlon.latitude;
        settings->Longitude = latlon.longitude;

        osm->clearMarkers();
        osm->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);

        if(gc->globe->isVisible()) {
            gc->clearMarkers();
            gc->globe->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onOperatorChanged()
{
    auto data = qsoPanel->getStationOperatorItems(qsoPanel->getStationOperator());
    userData.qsosu_operator_id = data.value(1).toInt();
    userData.oper = qsoPanel->getStationOperator();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onUdpLogged()
{
    QString Locator;

    qInfo() << "Creating database record for UDP message";

    if(udpServer->dx_call.length() == 0) {
        qDebug() << "UPD: Empty CallSign was received.";
        return;
    }

    if (udpServer->dx_grid.isEmpty())
        Locator = qsoPanel->getQTHLocator();
    else
        Locator = QString::fromUtf8(udpServer->dx_grid);
        ShowQSOLocation(udpServer->dx_call, Locator);

    QSqlRecord newRecord = RecordsModel->record();
    newRecord.remove(newRecord.indexOf("id"));
    newRecord.setValue("callsign_id", userData.callsign_id);
    newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
    newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
    newRecord.setValue("STATION_CALLSIGN", userData.callsign);
    newRecord.setValue("OPERATOR", userData.oper);
    newRecord.setValue("MY_GRIDSQUARE", qsoPanel->getStationQTHLocator());
    newRecord.setValue("MY_CNTY", qsoPanel->getStationRDA());
    newRecord.setValue("QSO_DATE", udpServer->time_off.date().toString("yyyyMMdd"));
    newRecord.setValue("TIME_OFF", udpServer->time_off.time().toString("hhmm") + "00");
    newRecord.setValue("TIME_ON", udpServer->time_on.time().toString("hhmm") + "00");
    newRecord.setValue("CALL", QString::fromUtf8(udpServer->dx_call));
    newRecord.setValue("BAND", Helpers::GetBandByFreqHz(udpServer->tx_frequency_hz).toUpper());
    newRecord.setValue("FREQ", udpServer->tx_frequency_hz);
    newRecord.setValue("MODE", QString::fromUtf8(udpServer->mode));
    newRecord.setValue("NAME", qsoPanel->getName());
    newRecord.setValue("QTH", qsoPanel->getQTH());
    newRecord.setValue("GRIDSQUARE", Locator);
    newRecord.setValue("CNTY", qsoPanel->getRDA());
    newRecord.setValue("RST_SENT", QString::fromUtf8(udpServer->report_sent));
    newRecord.setValue("RST_RCVD", QString::fromUtf8(udpServer->report_received));
    newRecord.setValue("COMMENT", QString::fromUtf8(udpServer->comments));
    newRecord.setValue("QSL_STATUS", 0);
    newRecord.setValue("sync_state", 0);

    CountryEntry result = findCountryByCall(udpServer->dx_call, ctyData);
    if(!result.country.isEmpty())
    {
        newRecord.setValue("COUNTRY", result.country);
        newRecord.setValue("COUNTRY_CODE", countryToIso.value(result.country, ""));
        newRecord.setValue("CONT", result.continent);
        newRecord.setValue("ITUZ", result.ituZone);
        newRecord.setValue("CQZ", result.cqZone);
    }

    qsoPanel->setCallsign(QString::fromUtf8(udpServer->dx_call));
    qsoPanel->setRSTR(QString::fromUtf8(udpServer->report_received));
    qsoPanel->setRSTS(QString::fromUtf8(udpServer->report_sent));
    qsoPanel->setQTHLocator(QString::fromUtf8(udpServer->dx_grid));
    qsoPanel->setComment(QString::fromUtf8(udpServer->comments));

    if(RecordsModel->insertRecord(-1, newRecord)) {
        if (RecordsModel->submitAll()) {
            qInfo() << "New record submited to database";

            int LastID = RecordsModel->query().lastInsertId().toInt();
            QVariantList data;
            QString datetime = udpServer->time_on.date().toString("yyyy-MM-dd") + "T" + udpServer->time_on.time().toString("hh:mm:00");
            data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
            data << QString::fromUtf8(udpServer->dx_call) << Helpers::GetBandByFreqHz(udpServer->tx_frequency_hz) << QString::fromUtf8(udpServer->mode);
            data << udpServer->tx_frequency_hz << datetime << QString::fromUtf8(udpServer->name) << QString::fromUtf8(udpServer->report_sent) << QString::fromUtf8(udpServer->report_received);
            data << "" << "" << Locator << qsoPanel->getStationQTHLocator() << qsoPanel->getStationRDA();

            if(api->serviceAvailable) api->SendQso(data);
            if(qrz->serviceAvailable) {
                data << userData.gridsquare << userData.cnty << userData.callsign << userData.oper;
                logradio->SendQso(data);
            }
            RefreshRecords();
            ScrollRecordsToTop();
            qsoPanel->clearQSO();
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onUdpLoggedADIF()
{
    QString Locator;

    qInfo() << "Creating database record for UDP ADIF message";

    if(udpServer->adifData.value("CALL") == 0) {
         qDebug() << "UPD ADIF: Empty CallSign was received.";
         return;
    }

    if (udpServer->adifData.value("GRIDSQUARE").isEmpty())
        Locator = qsoPanel->getQTHLocator();
    else
        Locator = udpServer->adifData.value("GRIDSQUARE");
        ShowQSOLocation(udpServer->adifData.value("CALL"), Locator);

    QSqlRecord newRecord = RecordsModel->record();
    newRecord.remove(newRecord.indexOf("id"));
    newRecord.setValue("callsign_id", userData.callsign_id);
    newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
    newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
    newRecord.setValue("STATION_CALLSIGN", userData.callsign);
    newRecord.setValue("OPERATOR", userData.oper);
    newRecord.setValue("MY_GRIDSQUARE", qsoPanel->getStationQTHLocator());
    newRecord.setValue("MY_CNTY", qsoPanel->getStationRDA());
    QDate qso_date = QDate::fromString(udpServer->adifData.value("QSO_DATE"), "yyyyMMdd");
    newRecord.setValue("QSO_DATE", udpServer->adifData.value("QSO_DATE"));
    newRecord.setValue("TIME_OFF", udpServer->adifData.value("TIME_OFF"));
    QTime time_on = QTime::fromString(udpServer->adifData.value("TIME_ON"), "hhmmss");
    newRecord.setValue("TIME_ON", udpServer->adifData.value("TIME_ON"));
    newRecord.setValue("CALL", udpServer->adifData.value("CALL"));
    newRecord.setValue("BAND", udpServer->adifData.value("BAND").toUpper());
    unsigned long long freqHz = static_cast<unsigned long long>(udpServer->adifData.value("FREQ").toDouble() * 1000000);
    newRecord.setValue("FREQ", freqHz);
    newRecord.setValue("MODE", udpServer->adifData.value("MODE"));
    newRecord.setValue("NAME", qsoPanel->getName());
    newRecord.setValue("QTH", qsoPanel->getQTH());
    newRecord.setValue("GRIDSQUARE", Locator);
    newRecord.setValue("CNTY", qsoPanel->getRDA());
    newRecord.setValue("RST_SENT", udpServer->adifData.value("RST_SENT"));
    newRecord.setValue("RST_RCVD", udpServer->adifData.value("RST_RCVD"));
    newRecord.setValue("COMMENT", udpServer->adifData.value("COMMENT"));
    newRecord.setValue("QSL_STATUS", 0);
    newRecord.setValue("sync_state", 0);

    CountryEntry result = findCountryByCall(udpServer->adifData.value("CALL"), ctyData);
    if(!result.country.isEmpty())
    {
        newRecord.setValue("COUNTRY", result.country);
        newRecord.setValue("COUNTRY_CODE", countryToIso.value(result.country, ""));
        newRecord.setValue("CONT", result.continent);
        newRecord.setValue("ITUZ", result.ituZone);
        newRecord.setValue("CQZ", result.cqZone);
    }

    qsoPanel->setCallsign(udpServer->adifData.value("CALL"));
    qsoPanel->setRSTR(udpServer->adifData.value("RST_RCVD"));
    qsoPanel->setRSTS(udpServer->adifData.value("RST_SENT"));
    qsoPanel->setQTHLocator(udpServer->adifData.value("GRIDSQUARE"));
    qsoPanel->setComment(udpServer->adifData.value("COMMENT"));

    if(RecordsModel->insertRecord(-1, newRecord)) {
        if (RecordsModel->submitAll()) {
            qInfo() << "New record submited to database";

            int LastID = RecordsModel->query().lastInsertId().toInt();
            QVariantList data;
            QString datetime = qso_date.toString("yyyy-MM-dd") + "T" + time_on.toString("hh:mm:00");
            data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
            data << udpServer->adifData.value("CALL") << udpServer->adifData.value("BAND") << udpServer->adifData.value("MODE") << freqHz;
            data << datetime << "" << udpServer->adifData.value("RST_SENT") << udpServer->adifData.value("RST_RCVD") << "" << "";
            data << Locator << qsoPanel->getStationQTHLocator() << qsoPanel->getStationRDA();

            if(api->serviceAvailable) api->SendQso(data);
            if(qrz->serviceAvailable) {
                data << userData.gridsquare << userData.cnty << userData.callsign << userData.oper;
                logradio->SendQso(data);
            }
            RefreshRecords();
            ScrollRecordsToTop();
            qsoPanel->clearQSO();
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::FindCallData(const QString &callsign)
{
    // Очистка, если позывной пустой или короткий
    if (callsign.isEmpty() || callsign.count() < 3) {
        qsoPanel->ClearCallbookFields();
        qsoPanel->setQSOSUserVisible(false);
        qsoPanel->setQSOSUserLabelVisible(false);
        qsoPanel->setUserSRRVisible(false);
        qsoPanel->setUserSRRLabelVisible(false);
        qsoPanel->setFlagVisible(false);
        qsoPanel->setCountryVisible(false);
        showPreviosQSO("");
        osm->clearMarkers();
        osm->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);

        if (gc->globe->isVisible()) {
            gc->clearMarkers();
            gc->globe->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
        }
        return;
    }

    CountryEntry result = findCountryByCall(callsign, ctyData);
    if (!result.country.isEmpty()) {
        qsoPanel->setFlagVisible(true);
        qsoPanel->setCountryVisible(true);
        qsoPanel->setFlag(countryToIso.value(result.country));
        qsoPanel->setCountry(result.country);
    }
    showPreviosQSO(callsign);

    lastCallsign = callsign;

    // нет интернета → сразу local
    if (!api->serviceAvailable && settings->useLocalCallbook) {
        findInLocal(callsign);
        return;
    }
    // есть интернет
    if (settings->useCallbook) {
        api->getCallbook(callsign);
        return;
    }

    if (settings->enableQrzruCallbook) {
        if (!findInQrz(callsign))
            if(settings->useLocalCallbook) findInLocal(callsign);
        return;
    }
    if(settings->useLocalCallbook) findInLocal(callsign);
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::findInLocal(const QString &callsign)
{
    QStringList data = localCallbook->searchCall(callsign);
    if (data.isEmpty()) return false;

    qsoPanel->setName(data.value(1));
    qsoPanel->setQTH(data.value(2));
    qsoPanel->setQTHLocator(data.value(3).toUpper());
    qsoPanel->setRDA(data.value(4).toUpper());
    ShowQSOLocation(callsign, data.value(3));

    qInfo() << "Local callbook:" << callsign;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::findInQrz(const QString &callsign)
{
    QStringList data = qrz->Get(callsign);
    if (data.isEmpty()) return false;

    qsoPanel->setName(data.value(0));
    qsoPanel->setQTH(data.value(1));
    qsoPanel->setQTHLocator(data.value(2).toUpper());
    qsoPanel->setRDA(data.value(3).toUpper());
    ShowQSOLocation(callsign, data.value(2));

    qInfo() << "QRZ.RU:" << callsign;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onQsoSuResult(bool found)
{
    // пользователь уже ввёл другой позывной
    if (qsoPanel->getCallsign() != lastCallsign) return;

    if (found) {
        setUserData();
        return;
    }
    // QSO.SU не дал данных → local
    if(settings->useLocalCallbook) findInLocal(lastCallsign);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setUserData()
{
    QStringList data;
    QString callsign, locator;

    data.append(api->callsignInfo);
    qsoPanel->setName((data.at(0).length() > 0) ? data.at(0) : "");
    qsoPanel->setQTH((data.at(1).length() > 0) ? data.at(1) : "");
    qsoPanel->setQTHLocator((data.at(2).length() > 0) ? data.at(2).toUpper() : "");
    qsoPanel->setRDA((data.at(3).length() > 0) ? data.at(3).toUpper() : "");

    if(data.at(4).length() > 0) {
        if(data.at(4) == "1") {
            qsoPanel->setQSOSUserPixmap(QPixmap(":resources/images/loguser.png"));
            qsoPanel->setQSOSUserVisible(true);
            qsoPanel->setQSOSUserLabelVisible(true);
            qsoPanel->setQSOSUserLabelText(tr("Пользователь QSO.SU"));
            qsoPanel->setQSOSUserLabelStyle("QLabel { font-weight: bold; color: rgb(25, 135, 84) }");
        } else {
            qsoPanel->setQSOSUserPixmap(QPixmap(":resources/images/no_loguser.png"));
            qsoPanel->setQSOSUserLabelText(tr("Не пользователь QSO.SU"));
            qsoPanel->setQSOSUserLabelStyle("QLabel { font-weight: bold; color: rgb(220, 53, 69) }");
        }
    } else {
        qsoPanel->setQSOSUserVisible(false);
        qsoPanel->setQSOSUserLabelVisible(false);
    }

    if(data.at(5).length() > 0) {
        if(data.at(5) == "1") {
            qsoPanel->setUserSRRPixmap(QPixmap(":resources/images/srr_user.png"));
            qsoPanel->setUserSRRVisible(true);
            qsoPanel->setUserSRRLabelVisible(true);
            qsoPanel->setUserSRRLabelText(tr("Член СРР"));
            qsoPanel->setUserSRRLabelStyle("QLabel { font-weight: bold; color: rgb(25, 135, 84) }");
        } else {
            qsoPanel->setUserSRRVisible(false);
            qsoPanel->setUserSRRLabelVisible(false);
        }
    } else {
        qsoPanel->setUserSRRVisible(false);
        qsoPanel->setUserSRRLabelVisible(false);
    }
    callsign = qsoPanel->getCallsign();
    qInfo() << "Use QSO.SU callbook. " << "CallSign: " << callsign << " Name: " << data.at(0) << " QTH: " << data.at(1) << " Locator: " << data.at(2) << " RDA:" << data.at(3);
    if(data.at(2).length() > 0) ShowQSOLocation(callsign, data.at(2));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::fillDefaultFreq()
{
    if(settings->catEnable) return; //Если включен CAT, то частоту берем из TRX
    double freqMhz = BandToDefaultFreq(qsoPanel->getBand()) * 1e6;
    qsoPanel->setFrequence(freqMhz);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SyncQSOs(QModelIndexList indexes)
{
    QStringList idstrings;
    for (int i = 0; i < indexes.size(); i++){
        idstrings << indexes[i].data().toString();
    }
    QString numberlist = idstrings.join(",");
    QSqlQuery query(db);
    query.exec(QString("SELECT id, qsosu_callsign_id, qsosu_operator_id, CALL, BAND, MODE, FREQ, QSO_DATE, TIME_OFF, NAME, RST_SENT, RST_RCVD, QTH, CNTY, GRIDSQUARE, MY_CNTY, MY_GRIDSQUARE FROM records WHERE id IN (%1) ORDER BY id").arg(numberlist));
    while (query.next()) {
        int dbid = query.value(0).toInt();
        QString call = query.value(3).toString();
        QString band = query.value(4).toString();
        QString mode = query.value(5).toString();
        int freqHz = query.value(6).toInt();
        QString date = QDate::fromString(query.value(7).toString(), "yyyyMMdd").toString("yyyy-MM-dd");
        QString time = QTime::fromString(query.value(8).toString(), (query.value(8).toString().length() == 6) ? "hhmmss" : "hhmm").toString("hh:mm:ss");
        QString datetime = date + "T" + time;
        QString name = query.value(9).toString();
        QString rsts = query.value(10).toString();
        QString rstr = query.value(11).toString();
        QString qth = query.value(12).toString();
        QString cnty = query.value(13).toString();
        QString gridsquare = query.value(14).toString();
        QString my_cnty = query.value(15).toString();
        QString my_gridsquare = query.value(16).toString();

        QVariantList data;
        data << dbid << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare << my_gridsquare << my_cnty;
        if(api->serviceAvailable) {
            api->SendQso(data);
            data << "" << "" << userData.callsign << userData.oper;
            if(settings->logRadioAccessToken.count() > 0) logradio->SendQso(data);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onQSOSUSynced(int dbid, QString hash)
{
    QSqlQuery query(db);

    uint8_t state = getSynchroStatus(dbid);
    state |= 1;
    query.prepare("UPDATE records SET sync_state = 0, SYNC_QSO = :nstate, HASH = :hash WHERE id=:id");
    query.bindValue(":hash", hash);
    query.bindValue(":id", dbid);
    query.bindValue(":nstate", state);

    if(!query.exec()){
        qDebug() << "ERROR UPDATE TABLE records FROM QSO.SU " << query.lastError().text();
    } else {
        RefreshRecords();
        ScrollRecordsToTop();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onLogRadioSynced(int dbid)
{
    QSqlQuery query(db);

    uint8_t state = getSynchroStatus(dbid);
    state |= (1 << 1);

    query.prepare("UPDATE records SET SYNC_QSO = :nstate WHERE id=:id");
    query.bindValue(":id", dbid);
    query.bindValue(":nstate", state);

    if(!query.exec()){
        qDebug() << "ERROR UPDATE TABLE records FROM LOGRADIO.RU " << query.lastError().text();
    } else {
        RefreshRecords();
        ScrollRecordsToTop();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::readXmlfile()
{
    QDomDocument HamDefs;
    QString basePath;

#ifdef Q_OS_MAC
    // В macOS: внутри .app → подняться на 3 уровня
    basePath = QCoreApplication::applicationDirPath() + "/HamDefs.xml";
#elif defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // В Windows и Linux файл лежит рядом с .exe
    basePath = QCoreApplication::applicationDirPath() + "/HamDefs.xml";
#else
    // fallback
    basePath = "HamDefs.xml";
#endif

    QFile xmlFile(basePath);

    if(!xmlFile.open(QIODevice::ReadOnly)) return false;

    if(!HamDefs.setContent(&xmlFile))
    {
        qDebug() << "Error read HamDefs.xml";
        return false;
    }

    QDomElement root = HamDefs.firstChildElement();
    QDomNodeList BandNames = HamDefs.elementsByTagName("band");

    for(int i = 0; i < BandNames.count(); i++)
    {
        QDomNode bandNode = BandNames.at(i);
        if(bandNode.isElement())
        {
            QDomElement Band = bandNode.toElement();
            bandData bData;
            bData.band_id = Band.attribute("id").toInt();
            bData.band_name = Band.attribute("band_name");
            bData.band_value = Band.attribute("band_value");
            bData.band_freq = Band.attribute("band_freq");
            bandList.append(bData);
            qsoPanel->addBandItems(Band.attribute("band_name"));
        }
    }

    QDomNodeList ModeNames = HamDefs.elementsByTagName("mode");

    for(int j = 0; j < ModeNames.count(); j++)
    {
        QDomNode modeNode = ModeNames.at(j);
        if(modeNode.isElement())
        {
            QDomElement Mode = modeNode.toElement();
            modeData mData;
            mData.mode_id = Mode.attribute("id").toInt();
            mData.mode_name = Mode.attribute("mode_name");
            mData.mode_value = Mode.attribute("mode_value");
            mData.mode_report = Mode.attribute("report");
            modeList.append(mData);
            qsoPanel->addModeItems(Mode.attribute("mode_name"));
        }
    }
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::LoadHamDefsSync()
{
    qInfo() << tr("Загрузка файла HamDefs.xml с сервера QSO.SU");

    QEventLoop loop;
    bool success = false;

    // Подписываемся на сигналы от api
    connect(api, &HttpApi::HamDefsUploaded, &loop, [&](){
        QDomDocument HamDefsDoc;
        HamDefsDoc.setContent(api->XMLdata);

        QFile file("HamDefs.xml");
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            HamDefsDoc.save(out, 0);
            file.close();
        }
        success = readXmlfile(); // читаем XML
        loop.quit();
    });

    connect(api, &HttpApi::HamDefsError, &loop, [&](){
        qInfo() << tr("Открытие файла HamDefs.xml с диска.");
        success = readXmlfile();
        loop.quit();
    });

    api->loadHamDefs(); // Запускаем загрузку с сервера
    loop.exec(); // Ждём завершения загрузки или ошибки
    return success; // true если файл успешно прочитан
}
//------------------------------------------------------------------------------------------------------------------------------------------

double MainWindow::BandToDefaultFreq(QString band)
{
    for(int j = 0; j < bandList.count(); j++)
    {
        if(bandList[j].band_name == band)
            return bandList[j].band_freq.toDouble();
    }
    return 0.0;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString MainWindow::getBandValue(int index)
{
    for(int j = 0; j < bandList.count(); j++)
    {
        if(bandList[j].band_id == index)
            return bandList[j].band_value;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString MainWindow::getModeValue(QString mode)
{
    for(int j = 0; j < modeList.count(); j++)
    {
        if(modeList[j].mode_name == mode)
            return modeList[j].mode_value;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString MainWindow::getRepotValueFromMode(QString mode)
{
    for(int j = 0; j < modeList.count(); j++)
    {
        if(mode == modeList[j].mode_name)
            return modeList[j].mode_report;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::lightTheme()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    qApp->setPalette(qApp->style()->standardPalette());
    qApp->setStyleSheet("");
    qApp->style()->polish(qApp);
    QApplication::processEvents();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::darkTheme()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("");
    qApp->style()->polish(qApp);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::insertDataToDeferredQSOs(int idx, QString hash)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO DELRECORDS(id, HASH) VALUES(?, ?)");
    query.addBindValue(idx);
    query.addBindValue(hash);

    qInfo() << QString::number(idx) << hash;

    if(!query.exec()){
        qDebug() << "ERROR insert data into deferred QSOs... " << query.lastError().text();
     } else {
        qDebug() << "Insert record " << QString::number(idx) << " into deferred QSOs.";
     }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RemoveDeferredQSOs()
{
    int rowcount = 0;

    qInfo() << "Remove Deferred QSOs.";
    db.transaction();
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM DELRECORDS");
    query.exec(); query.next();
    rowcount = query.value(0).toInt();

    query.prepare("SELECT HASH FROM DELRECORDS");
    query.exec(); query.next();

    for(int i = rowcount; i > 0; i--) {
        api->deleteByHashLog(query.value(0).toString());
        qInfo() << "Del QSO: " << query.value(0).toString();
        query.next();
    }
    query.prepare("DELETE FROM DELRECORDS");
    query.exec();
    db.commit();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::EditQSO(QModelIndex index)
{
    int idx = index.row();
    int dbid = RecordsModel->data(RecordsModel->index(idx, 0)).toInt();
    int qsosu_callsign_id = RecordsModel->data(RecordsModel->index(idx, 2)).toInt();
    int qsosu_operator_id = RecordsModel->data(RecordsModel->index(idx, 3)).toInt();
    QString call = RecordsModel->data(RecordsModel->index(idx, 8)).toString();
    QString date = RecordsModel->data(RecordsModel->index(idx, 9)).toString();
    QString time_start = RecordsModel->data(RecordsModel->index(idx, 10)).toString();
    QString time_stop = RecordsModel->data(RecordsModel->index(idx, 11)).toString();
    QString band = RecordsModel->data(RecordsModel->index(idx, 12)).toString();
    QString mode = RecordsModel->data(RecordsModel->index(idx, 14)).toString();
    QString freq = RecordsModel->data(RecordsModel->index(idx, 13)).toString();
    QString name = RecordsModel->data(RecordsModel->index(idx, 17)).toString();
    QString qth = RecordsModel->data(RecordsModel->index(idx, 18)).toString();
    QString rstr = RecordsModel->data(RecordsModel->index(idx, 16)).toString();
    QString rsts = RecordsModel->data(RecordsModel->index(idx, 15)).toString();
    QString locator = RecordsModel->data(RecordsModel->index(idx, 19)).toString();
    QString rda = RecordsModel->data(RecordsModel->index(idx, 20)).toString();
    QString ituz = RecordsModel->data(RecordsModel->index(idx, 23)).toString();
    QString cqz = RecordsModel->data(RecordsModel->index(idx, 24)).toString();
    QString country = RecordsModel->data(RecordsModel->index(idx, 26)).toString();
    QString country_code = RecordsModel->data(RecordsModel->index(idx, 27)).toString();
    QString comment = RecordsModel->data(RecordsModel->index(idx, 29)).toString();

    QVariantList data;
    data.clear();
    data << dbid << qsosu_callsign_id << qsosu_operator_id << call << date << time_start << time_stop << band
         << mode << freq << name << qth << rstr << rsts << locator << rda << ituz << cqz << comment << country << country_code;
    qsoedit->ShowQSOParams(data);
    qsoedit->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::doubleClickedQSO(QModelIndex idx)
{
    EditQSO(idx);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::doubleClickedPrevQSO(QModelIndex idx)
{
    if (!idx.isValid()) return;

    int row = idx.row();
    int id = PrevRecordsModel->data(PrevRecordsModel->index(row, 0)).toInt(); // столбец 0 — id

    QModelIndex index = findIndexById(RecordsModel, id);
    if (index.isValid()) {
        EditQSO(index);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::PingQsoSu()
{
    api->getPing();
    if(api->serviceAvailable) {
        qsosuLabel->setText("Online");
        qsosuLabel->setStyleSheet("QLabel { color: green }");
    } else {
        qsosuLabel->setText("Offline");
        qsosuLabel->setStyleSheet("QLabel { color: red }");
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::PingQrzRu()
{
    qrz->pingQrzRu();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setFreq(long freq)
{
    settings->lastFrequence = QString::number((double) freq / 1000000, 'f', 6);
    qsoPanel->setFrequenceText(settings->lastFrequence);
    //qDebug() << "CAT Freq:" << settings->lastFrequence;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setBand(int band)
{
    settings->lastBand = bandList[band].band_name;
    qsoPanel->setBand(bandList[band].band_name);
    //qDebug() << "CAT Band:" << bandList[band].band_name;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setMode(int mode)
{
    settings->lastMode = modeList[mode].mode_name;
    qsoPanel->setMode(modeList[mode].mode_name);
    //qDebug() << "CAT Mode:" << modeList[mode].mode_name;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onModeChanged(QString mode)
{
   QString str = getRepotValueFromMode(mode);
   qsoPanel->setRSTR(str);
   qsoPanel->setRSTS(str);
}
//------------------------------------------------------------------------------------------------------------------------------------------

int MainWindow::getSynchroStatus(int id)
{
    int state = 0;

    QSqlQuery query(db);
    query.exec(QString("SELECT SYNC_QSO FROM records WHERE id = %1").arg(id));

    while(query.next()){
        state = query.value(0).toInt();
    }
    return state;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionConfirmQSOs_triggered()
{
    confirmQSO = new ConfirmQSO(db, settings);
    connect(confirmQSO, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));
    confirmQSO->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionShowMap_triggered()
{
    osm->show();
    osm->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ShowQSOInMap()
{
    int count = 0;

    QSqlQuery query(db);
    query.exec(QString("SELECT CALL, GRIDSQUARE FROM records"));
    while (query.next()) {
        QString call = query.value(0).toString();
        QString gridsquare = query.value(1).toString();

        if(gridsquare.length() > 0) {
            Coordinates latlon = osm->locatorToCoordinates(gridsquare);
            count++;
            osm->setQSOMarker(call, latlon.latitude, latlon.longitude);
            qInfo() << "id: " << count << "Call: " << call << " Gridsquare: " << gridsquare << " Lat: " << QString::number(latlon.latitude) << " Lng: " << QString::number(latlon.longitude);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    if (index.isValid()) {
        QModelIndex targetIndex = ui->tableView->model()->index(index.row(), 8);
        QVariant call = ui->tableView->model()->data(targetIndex);
        if(ui->prevQSOtableView->isVisible()) showPreviosQSO(call.toString());
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::showPreviosQSO(QString call)
{
    if (call.trimmed().isEmpty()) {
        PrevRecordsModel->setFilter("1=0");
        PrevRecordsModel->select();
        return;
    }

    QStringList filters;
    filters << QString("CALL = '%1'").arg(call.replace("'", "''"));
    filters << QString("STATION_CALLSIGN = '%1'").arg(qsoPanel->getStationCallsign().replace("'", "''"));
    QString filterString = filters.join(" AND ");
    PrevRecordsModel->setFilter(filterString);
    PrevRecordsModel->setSort(PrevRecordsModel->fieldIndex("QSO_DATE"), Qt::DescendingOrder);
    PrevRecordsModel->select();

    QHeaderView *horizontalHeader = ui->prevQSOtableView->horizontalHeader();
    horizontalHeader->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader->setMinimumSectionSize(32);
    horizontalHeader->setStretchLastSection(true);
    horizontalHeader->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    ui->prevQSOtableView->resizeColumnsToContents();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitPreviosQSOModel()
{
    PrevRecordsModel = new ColorSqlTableModel(this);
    PrevRecordsModel->setTable("records");
    PrevRecordsModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
    PrevRecordsModel->setHeaderData(4, Qt::Horizontal, tr("Станция"));
    PrevRecordsModel->setHeaderData(5, Qt::Horizontal, tr("Оператор"));
    PrevRecordsModel->setHeaderData(6, Qt::Horizontal, tr("Локатор станции"));
    PrevRecordsModel->setHeaderData(7, Qt::Horizontal, tr("CNTY станции"));
    PrevRecordsModel->setHeaderData(8, Qt::Horizontal, tr("Позывной"));
    PrevRecordsModel->setHeaderData(9, Qt::Horizontal, tr("Дата"));
    PrevRecordsModel->setHeaderData(10, Qt::Horizontal, tr("Время нач."));
    PrevRecordsModel->setHeaderData(11, Qt::Horizontal, tr("Время оконч."));
    PrevRecordsModel->setHeaderData(12, Qt::Horizontal, tr("Диапазон"));
    PrevRecordsModel->setHeaderData(13, Qt::Horizontal, tr("Частота"));
    PrevRecordsModel->setHeaderData(14, Qt::Horizontal, tr("Мода"));
    PrevRecordsModel->setHeaderData(15, Qt::Horizontal, tr("RST отпр."));
    PrevRecordsModel->setHeaderData(16, Qt::Horizontal, tr("RST прин."));
    PrevRecordsModel->setHeaderData(17, Qt::Horizontal, tr("Имя"));
    PrevRecordsModel->setHeaderData(18, Qt::Horizontal, tr("QTH"));
    PrevRecordsModel->setHeaderData(19, Qt::Horizontal, tr("Локатор"));
    PrevRecordsModel->setHeaderData(20, Qt::Horizontal, tr("Район"));
    PrevRecordsModel->setHeaderData(21, Qt::Horizontal, tr("CFM"));
    PrevRecordsModel->setHeaderData(23, Qt::Horizontal, tr("ITUZ"));
    PrevRecordsModel->setHeaderData(24, Qt::Horizontal, tr("CQZ"));
    PrevRecordsModel->setHeaderData(26, Qt::Horizontal, tr("Территория"));
    PrevRecordsModel->setHeaderData(27, Qt::Horizontal, tr("Континент"));
    PrevRecordsModel->setHeaderData(28, Qt::Horizontal, tr("Флаг"));
    PrevRecordsModel->setHeaderData(29, Qt::Horizontal, tr("Коммент."));

    ui->prevQSOtableView->setModel(PrevRecordsModel);
    ui->prevQSOtableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if(testbit(settings->table_row_state, 0)) ui->prevQSOtableView->setColumnHidden(0, false); //ID
    else ui->prevQSOtableView->setColumnHidden(0, true);
    ui->prevQSOtableView->setColumnHidden(1, true);
    ui->prevQSOtableView->setColumnHidden(2, true);
    ui->prevQSOtableView->setColumnHidden(3, true);
    ui->prevQSOtableView->setColumnHidden(30, true);
    if(testbit(settings->table_row_state, 4)) ui->prevQSOtableView->setColumnHidden(4, false); //Station Callsign
    else ui->prevQSOtableView->setColumnHidden(4, true);
    if(testbit(settings->table_row_state, 5)) ui->prevQSOtableView->setColumnHidden(5, false); //Operator
    else ui->prevQSOtableView->setColumnHidden(5, true);
    if(testbit(settings->table_row_state, 6)) ui->prevQSOtableView->setColumnHidden(6, false); //MY GRIDSQUARE
    else ui->prevQSOtableView->setColumnHidden(6, true);
    if(testbit(settings->table_row_state, 7)) ui->prevQSOtableView->setColumnHidden(7, false); //MY CNTY
    else ui->prevQSOtableView->setColumnHidden(7, true);
    if(testbit(settings->table_row_state, 9)) ui->prevQSOtableView->setColumnHidden(9, false); //QSO Date
    else ui->prevQSOtableView->setColumnHidden(9, true);
    if(testbit(settings->table_row_state, 10)) ui->prevQSOtableView->setColumnHidden(10, false); //Time On
    else ui->prevQSOtableView->setColumnHidden(10, true);
    if(testbit(settings->table_row_state, 11)) ui->prevQSOtableView->setColumnHidden(11, false); //Time Off
    else ui->prevQSOtableView->setColumnHidden(11, true);
    if(testbit(settings->table_row_state, 13)) ui->prevQSOtableView->setColumnHidden(13, false); //Freq
    else ui->prevQSOtableView->setColumnHidden(13, true);
    if(testbit(settings->table_row_state, 15)) ui->prevQSOtableView->setColumnHidden(15, false); //RST Sent
    else ui->prevQSOtableView->setColumnHidden(15, true);
    if(testbit(settings->table_row_state, 16)) ui->prevQSOtableView->setColumnHidden(16, false); //RST Rcvd
    else ui->prevQSOtableView->setColumnHidden(16, true);
    if(testbit(settings->table_row_state, 17)) ui->prevQSOtableView->setColumnHidden(17, false); //Name
    else ui->prevQSOtableView->setColumnHidden(17, true);
    if(testbit(settings->table_row_state, 18)) ui->prevQSOtableView->setColumnHidden(18, false); //QTH
    else ui->prevQSOtableView->setColumnHidden(18, true);
    if(testbit(settings->table_row_state, 19)) ui->prevQSOtableView->setColumnHidden(19, false); //GRIDSQUARE
    else ui->prevQSOtableView->setColumnHidden(19, true);
    if(testbit(settings->table_row_state, 20)) ui->prevQSOtableView->setColumnHidden(20, false); //CNTY
    else ui->prevQSOtableView->setColumnHidden(20, true);
    ui->prevQSOtableView->setColumnHidden(22, true);
    if(testbit(settings->table_row_state, 23)) ui->prevQSOtableView->setColumnHidden(23, false); //ITUZ
    else ui->prevQSOtableView->setColumnHidden(23, true);
    if(testbit(settings->table_row_state, 24)) ui->prevQSOtableView->setColumnHidden(24, false); //CQZ
    else ui->prevQSOtableView->setColumnHidden(24, true);
    ui->prevQSOtableView->setColumnHidden(25, true);
    if(testbit(settings->table_row_state, 26)) ui->prevQSOtableView->setColumnHidden(26, false); //Country
    else ui->prevQSOtableView->setColumnHidden(26, true);
    if(testbit(settings->table_row_state, 27)) ui->prevQSOtableView->setColumnHidden(27, false); //Cont
    else ui->prevQSOtableView->setColumnHidden(27, true);
    if(testbit(settings->table_row_state, 28)) ui->prevQSOtableView->setColumnHidden(28, false); //Country code
    else ui->prevQSOtableView->setColumnHidden(28, true);
    if(testbit(settings->table_row_state, 29)) ui->prevQSOtableView->setColumnHidden(29, false); //Comment
    else ui->prevQSOtableView->setColumnHidden(29, true);

    ui->prevQSOtableView->setItemDelegateForColumn(8, new FormatCallsign(ui->prevQSOtableView));
    ui->prevQSOtableView->setItemDelegateForColumn(9, new FormatDate(ui->prevQSOtableView));
    ui->prevQSOtableView->setItemDelegateForColumn(10, new FormatTime(ui->prevQSOtableView));
    ui->prevQSOtableView->setItemDelegateForColumn(11, new FormatTime(ui->prevQSOtableView));
    ui->prevQSOtableView->setItemDelegateForColumn(13, new FormatFreq(ui->prevQSOtableView));
    ui->prevQSOtableView->setItemDelegateForColumn(21, new FormatSyncState(ui->prevQSOtableView));
    ui->prevQSOtableView->horizontalHeader()->moveSection(21, 0);
    ui->prevQSOtableView->horizontalHeader()->moveSection(28, 9);
    ui->prevQSOtableView->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");

    QHeaderView *verticalHeader = ui->prevQSOtableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(20);

    ui->prevQSOtableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->prevQSOtableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->prevQSOtableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionPreviosQSO_triggered(bool checked)
{
    auto model = ui->tableView->model();

    if(checked) ui->prevQSOtableView->setVisible(true);
    else ui->prevQSOtableView->setVisible(false);

    QModelIndex index = model->index(0, 1);
    if (index.isValid())
    {
        ui->tableView->setCurrentIndex(index);
        ui->tableView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

        QModelIndex targetIndex = ui->tableView->model()->index(index.row(), 8);
        QVariant call = ui->tableView->model()->data(targetIndex);
        if(ui->prevQSOtableView->isVisible()) showPreviosQSO(call.toString());
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setLanguage()
{
    // Сначала удаляем предыдущий переводчик
    qApp->removeTranslator(&qtLanguageTranslator);

    QString lang = settings->language;

    if (lang == "Русский") {
        ui->retranslateUi(this);
        return;
    }
    // Для остальных языков ищем соответствующий .qm файл
    QString path = QCoreApplication::applicationDirPath() + "/translations";
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Каталог translations не найден";
        return;
    }
    QStringList filters;
    filters << "*.qm";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    QString qmFile;

    for (const QFileInfo &fi : fileList) {
        QString base = fi.baseName();           // en_US, de_DE, ...
        QString langCode = base.split("_").first(); // en, de, fr, ...

        // Сравниваем красивое название с settings->language
        if ((langCode == "en" && lang == "English") ||
            (langCode == "de" && lang == "Deutsch") ||
            (langCode == "fr" && lang == "Français") ||
            (langCode == "es" && lang == "Español"))
        {
            qmFile = fi.absoluteFilePath();
            break;
        }
    }
    if (!qmFile.isEmpty() && QFile::exists(qmFile)) {
        if (qtLanguageTranslator.load(qmFile)) {
            qApp->installTranslator(&qtLanguageTranslator);
            qDebug() << "Загружен перевод:" << qmFile;
            ui->retranslateUi(this);
        } else {
            qDebug() << "Не удалось загрузить файл перевода:" << qmFile;
        }
    } else {
        qDebug() << "Файл перевода не найден для языка:" << lang;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionUploadQSOs_triggered()
{
    uploadLogs = new UploadingLogs(db, settings, ctyData);
    connect(uploadLogs, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));
    uploadLogs->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::loadCallList()
{
    QStringList CallList;

    QSqlQuery query;
    if (query.exec("SELECT DISTINCT CALL FROM RECORDS")) {
        while (query.next()) {
            QString callValue = query.value(0).toString();
            CallList.append(callValue);
        }
    } else {
        qDebug() << tr("Ошибка выполнения запроса: ") << query.lastError().text();
    }
    QCompleter *completer = new QCompleter(CallList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    qsoPanel->setCallsignCompleter(completer);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setTableRow()
{
    if(testbit(settings->table_row_state, 0)) ui->tableView->setColumnHidden(0, false); //ID
    else ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, true);
    ui->tableView->setColumnHidden(2, true);
    ui->tableView->setColumnHidden(3, true);
    ui->tableView->setColumnHidden(30, true);
    if(testbit(settings->table_row_state, 4)) ui->tableView->setColumnHidden(4, false); //Station Callsign
    else ui->tableView->setColumnHidden(4, true);
    if(testbit(settings->table_row_state, 5)) ui->tableView->setColumnHidden(5, false); //Operator
    else ui->tableView->setColumnHidden(5, true);
    if(testbit(settings->table_row_state, 6)) ui->tableView->setColumnHidden(6, false); //MY GRIDSQUARE
    else ui->tableView->setColumnHidden(6, true);
    if(testbit(settings->table_row_state, 7)) ui->tableView->setColumnHidden(7, false); //MY CNTY
    else ui->tableView->setColumnHidden(7, true);
    if(testbit(settings->table_row_state, 9)) ui->tableView->setColumnHidden(9, false); //QSO Date
    else ui->tableView->setColumnHidden(9, true);
    if(testbit(settings->table_row_state, 10)) ui->tableView->setColumnHidden(10, false); //Time On
    else ui->tableView->setColumnHidden(10, true);
    if(testbit(settings->table_row_state, 11)) ui->tableView->setColumnHidden(11, false); //Time Off
    else ui->tableView->setColumnHidden(11, true);
    if(testbit(settings->table_row_state, 13)) ui->tableView->setColumnHidden(13, false); //Freq
    else ui->tableView->setColumnHidden(13, true);
    if(testbit(settings->table_row_state, 15)) ui->tableView->setColumnHidden(15, false); //RST Sent
    else ui->tableView->setColumnHidden(15, true);
    if(testbit(settings->table_row_state, 16)) ui->tableView->setColumnHidden(16, false); //RST Rcvd
    else ui->tableView->setColumnHidden(16, true);
    if(testbit(settings->table_row_state, 17)) ui->tableView->setColumnHidden(17, false); //Name
    else ui->tableView->setColumnHidden(17, true);
    if(testbit(settings->table_row_state, 18)) ui->tableView->setColumnHidden(18, false); //QTH
    else ui->tableView->setColumnHidden(18, true);
    if(testbit(settings->table_row_state, 19)) ui->tableView->setColumnHidden(19, false); //GRIDSQUARE
    else ui->tableView->setColumnHidden(19, true);
    if(testbit(settings->table_row_state, 20)) ui->tableView->setColumnHidden(20, false); //CNTY
    else ui->tableView->setColumnHidden(20, true);
    ui->tableView->setColumnHidden(22, true);
    if(testbit(settings->table_row_state, 23)) ui->tableView->setColumnHidden(23, false); //ITUZ
    else ui->tableView->setColumnHidden(23, true);
    if(testbit(settings->table_row_state, 24)) ui->tableView->setColumnHidden(24, false); //CQZ
    else ui->tableView->setColumnHidden(24, true);
    ui->tableView->setColumnHidden(25, true);
    if(testbit(settings->table_row_state, 26)) ui->tableView->setColumnHidden(26, false); //Country
    else ui->tableView->setColumnHidden(26, true);
    if(testbit(settings->table_row_state, 27)) ui->tableView->setColumnHidden(27, false); //Cont
    else ui->tableView->setColumnHidden(27, true);
    if(testbit(settings->table_row_state, 28)) ui->tableView->setColumnHidden(28, false); //Country code
    else ui->tableView->setColumnHidden(28, true);
    if(testbit(settings->table_row_state, 29)) ui->tableView->setColumnHidden(29, false); //Comment
    else ui->tableView->setColumnHidden(29, true);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionUpdatePrefix_triggered()
{
    update_prefix->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionCheckCountry_triggered()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(QObject::tr("Определение территории по позывному:"));
    dialog.setLabelText(QObject::tr("Введите позывной:"));
    dialog.setTextValue("");
    dialog.setInputMode(QInputDialog::TextInput);

    // Убираем лишние кнопки в заголовке
    dialog.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Находим QLineEdit внутри диалога и вешаем валидатор
    if (QLineEdit *lineEdit = dialog.findChild<QLineEdit *>()) {
        auto *validator = new QRegularExpressionValidator(QRegularExpression("^[A-Za-z0-9/]*$"), lineEdit);
        lineEdit->setValidator(validator);

        // Автоматический апперкас (для позывных принято в верхнем регистре)
        QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [lineEdit](const QString &text){
            QString upper = text.toUpper();
            if (text != upper) {
                int pos = lineEdit->cursorPosition();
                lineEdit->setText(upper);
                lineEdit->setCursorPosition(pos);
            }
        });
    }

    // запускаем диалог
    if (dialog.exec() == QDialog::Accepted) {
        QString Call = dialog.textValue();

        CountryEntry result = findCountryByCall(Call, ctyData);

        if(!result.country.isEmpty())
        {
            QMessageBox::information(this, tr("Проверка территории по позывному"),
                                     tr("Позывной: ") + Call + "\n" +
                                     tr("Страна: ") + result.country + "\n" +
                                     tr("Континент: ") + result.continent + "\n" +
                                     tr("Префикс DXCC: ") + result.mainPrefix + "\n" +
                                     tr("Разница во времени (UTC):") + QString::number(result.utcOffset) + "\n" +
                                     tr("CQZ: ") + QString::number(result.cqZone) + "\n" +
                                     tr("ITUZ: ") + QString::number(result.ituZone));

            QString iso = countryToIso.value(result.country, "");
            QString path = ":/resources/flags/" + iso + ".png"; // используем ресурсы Qt
            if (!QFile::exists(path)) {
                 qDebug() << "Нет файла с флагом " << iso;
                 return;
            }
        } else {
            QMessageBox::information(this, tr("Проверка территории по позывному"), tr("Страна не найдена для позывного: ") + Call);
            qDebug() << tr("Страна не найдена для позывного: ") << Call;
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportCountry_triggered()
{
    reportCountry = new ReportCountry(db);
    reportCountry->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportContinent_triggered()
{
    reportContinent = new ReportContinent(db);
    reportContinent->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportSun_triggered()
{
    reportSunChart->show();

    QSqlQuery query(db);

    // Берём самое новое измерение
    query.prepare("SELECT MAX(MEASUREMENT_TIME) FROM magnetic_storm");
    if (!query.exec() || !query.next()) {
        qWarning() << "Не удалось получить дату последних данных";
        api->getMagneticStormHistory();
        reportSunChart->showMagneticStormChart();
        return;
    }

    QDateTime lastDt;
    if (!query.value(0).isNull()) {
        lastDt = QDateTime::fromString(
            query.value(0).toString(),
            Qt::ISODate
        );
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const bool needUpdate = !lastDt.isValid() || lastDt.secsTo(now) > 3 * 3600;

    if (needUpdate) {
        qInfo() << "Данные магнитной бури старше 3 часов — обновляем";
        query.exec("DELETE FROM magnetic_storm");
        api->getMagneticStormHistory();
    } else {
        qInfo() << "Данные свежие — используем кэш";
    }

    reportSunChart->showMagneticStormChart();
    MagStormUpdate();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QModelIndex MainWindow::findIndexById(QSqlTableModel *model, int id, int targetColumn)
{
    // Предполагается, что столбец с id — это 0-й
    const int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QVariant idValue = model->data(model->index(row, 0)); // столбец id
        if (idValue.toInt() == id) {
            // Возвращаем индекс в указанном столбце этой строки
            return model->index(row, targetColumn);
        }
    }
    return QModelIndex(); // если не найден
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::MagStormUpdate()
{
    if(api->serviceAvailable && settings->accessToken.length() != 0) {
        api->getMagneticStormCurrent();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::UpdateMeasurement(QJsonObject data)
{
    if (!data.isEmpty()) {
        int level = data.value("index").toInt();
        magStormLabel->setText(QString(tr("Уровень МБ: %1")).arg(level));
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportBands_triggered()
{
    reportBands = new ReportBands(db);
    reportBands->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportModes_triggered()
{
    reportModes = new ReportModes(db);
    reportModes->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionReportYear_triggered()
{
    reportYear = new ReportYear(db);
    reportYear->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::showBellIcon()
{
    bellBtn->setIcon(QIcon(":/resources/images/bell_new.png"));
    bellBtn->setToolTip(tr("Есть новые сообщения"));
    hasNewNews = true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::showNotificationIcon()
{
    notificationBtn->setIcon(QIcon(":/resources/images/notification_new.png"));
    notificationBtn->setToolTip(tr("Есть новые сообщения в чате"));
    hasNewMessages = true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::openNewsWindow()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Лента новостей"));
    dlg.setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dlg.resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QTextEdit *textEdit = new QTextEdit(&dlg);
    textEdit->setReadOnly(true);

    // Определяем текущую тему (светлая или тёмная)
    QPalette pal = QApplication::palette();
    bool darkTheme = pal.color(QPalette::Window).lightness() < 128;

    QString dateColor = darkTheme ? "#AAAAAA" : "#555555";
    QString titleColor = darkTheme ? "#FFFFFF" : "#000000";
    QString textColor = darkTheme ? "#DDDDDD" : "#222222";
    QString hrColor   = darkTheme ? "#666666" : "#CCCCCC";
    QString bgColor   = darkTheme ? "#2B2B2B" : "#FDFDFD";

    textEdit->setStyleSheet(QString("background-color: %1;").arg(bgColor));

    // Загружаем данные из базы
    QSqlQuery q("SELECT date, title, text FROM news ORDER BY id");
    QString htmlContent;

    while (q.next()) {
        QString dateStr = q.value(0).toString();
        QString title = q.value(1).toString();
        QString text = q.value(2).toString();

        QString newsHtml;
        newsHtml += "<div style='margin-bottom:14px;'>";
        // Дата
        newsHtml += QString("<p style='color:%1; font-size:10pt; margin:0;'>%2</p>")
                .arg(dateColor, dateStr);
        // Заголовок
        newsHtml += QString("<p style='color:%1; font-weight:bold; font-size:12pt; margin:2px 0;'>%2</p>")
                .arg(titleColor, title);
        // Текст новости
        newsHtml += QString("<p style='color:%1; font-size:10pt; margin:2px 0 5px 0;'>%2</p>")
                .arg(textColor, text);
        // Горизонтальная линия
        newsHtml += QString("<hr style='border:1px solid %1;'>").arg(hrColor);
        newsHtml += "</div>";
        // Добавляем новость в начало текста
        htmlContent = newsHtml + htmlContent;
    }
    textEdit->setHtml(htmlContent);
    layout->addWidget(textEdit);
    dlg.setLayout(layout);
    dlg.exec();

    // После просмотра сбрасываем иконку
    if (hasNewNews) {
        bellBtn->setIcon(QIcon(":/resources/images/bell.png"));
        bellBtn->setToolTip(tr("Нет новых уведомлений"));
        hasNewNews = false;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionShowSpots_triggered()
{
    if(spotServerConnected) spotViewer->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::saveHeaderState(QTableView *tableView)
{
    QSettings settings("QSO.SU", "QSOLogger");
    settings.setValue("headerStateMain", tableView->horizontalHeader()->saveState());
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::restoreHeaderState(QTableView *tableView)
{
    QSettings settings("QSO.SU", "QSOLogger");
    QByteArray state = settings.value("headerStateMain").toByteArray();
    if (!state.isEmpty()) {
        tableView->horizontalHeader()->restoreState(state);
    }
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::setSpotInfo(QString call, QString band, double freq, QString mode)
{
    qsoPanel->setCallsign(call);
    qsoPanel->setBand(band);
    qsoPanel->setFrequenceText(QString::number(freq / 1000000.0, 'f', 6));
    qsoPanel->setMode(mode);
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::on_actionImportADIF_triggered()
{
    imp_adif = new ImportADIF(db, ctyData);
    connect(imp_adif, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));
    imp_adif->show();
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::on_actionExportADIF_triggered()
{
    exp_adif = new ExportADIF(db);
    exp_adif->show();
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::on_actionChats_triggered()
{
    if(spotServerConnected) chats->show();

    // После просмотра сбрасываем иконку
    if (hasNewMessages) {
        notificationBtn->setIcon(QIcon(":/resources/images/notification.png"));
        notificationBtn->setToolTip(tr("Нет новых сообщений в чате"));
        hasNewMessages = false;
    }
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::on_actionShowLogLocation_triggered()
{
    // Получаем путь к папке с конфигами/логами
    QString folderPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    // Проверяем, существует ли папка
    if (QDir(folderPath).exists()) {
        // Открываем папку в файловом менеджере
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
    } else {
        qDebug() << "Папка не существует:" << folderPath;
        QMessageBox::warning(this, tr("Ошибка"), tr("Папка с логами не найдена."));
    }
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::SendSpotQSO(QModelIndex index)
{
    int idx = index.row();
    QString hash = RecordsModel->data(RecordsModel->index(idx, 22)).toString();

    // Получаем дату QSO из модели, формат "20251111"
    QString qsoDateStr = RecordsModel->data(RecordsModel->index(idx, 9)).toString().trimmed();
    QDate qsoDate = QDate::fromString(qsoDateStr, "yyyyMMdd"); // <- правильный формат!

    // Получаем время QSO из модели, формат "HHmmss"
    QString qsoTimeStr = RecordsModel->data(RecordsModel->index(idx, 10)).toString().trimmed();
    QTime qsoTime = QTime::fromString(qsoTimeStr, "HHmmss");

    // Валидация
    if (!qsoDate.isValid()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный формат даты QSO: %1").arg(qsoDateStr));
        qWarning() << "Invalid QSO date string:" << qsoDateStr << "at row" << idx;
        return;
    }

    if (!qsoTime.isValid()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный формат времени QSO: %1").arg(qsoTimeStr));
        qWarning() << "Invalid QSO time string:" << qsoTimeStr << "at row" << idx;
        return;
    }

    // Построим QDateTime из даты и времени из записи
    QDateTime qsoDateTime(qsoDate, qsoTime);

    qsoDateTime = QDateTime(qsoDate, qsoTime, Qt::UTC);
    QDateTime now = QDateTime::currentDateTimeUtc();

    if (qsoDateTime > now && qsoDateTime.secsTo(now) < -60*60*12) {
        // слишком далеко в будущем — логируем это как подозрительное
        qWarning() << "QSO datetime is in the future (suspicious):" << qsoDateTime.toString(Qt::ISODate);
    }

    // Разница в минутах (положительная, если QSO в прошлом)
    qint64 minutesPassed = qsoDateTime.secsTo(now) / 60;

    // Диагностический лог
    qInfo() << "Minutes passed since QSO:" << minutesPassed;

    if (minutesPassed > 15) {
        QMessageBox::warning(this, tr("Ошибка"),
                             tr("С момента проведения QSO прошло более 15 минут.\nОтправка спота невозможна."));
        qWarning() << "QSO too old:" << minutesPassed << "минут назад, время QSO:" << qsoTimeStr;
        return;
    }

    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Ввод комментария:"));
    dialog.setLabelText(tr("Введите комментарий к споту:"));
    dialog.setTextValue("");
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    if (dialog.exec() == QDialog::Accepted) {
        QString message = dialog.textValue();
        api->sendSpot(hash, message);
        qInfo() << "Sended Spot: HASH: " << hash << " Message: " << message;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ShowQSOLocation(QString callsign, QString locator)
{
    Coordinates latlon;
    double azimuth, rev_azimuth, distance;

    if (locator.length() > 3) {
        latlon = osm->locatorToCoordinates(locator);
        osm->clearMarkers();
        osm->showQSOMap(callsign, latlon.latitude, latlon.longitude);
        if(gc->globe->isVisible()) {
            gc->clearMarkers();
            gc->globe->showQSO(callsign, latlon.latitude, latlon.longitude);
            gc->setLatLonText(tr("Широта: %1'N  Долгота: %2'E").arg(latlon.latitude, 0, 'f', 2).arg(latlon.longitude, 0, 'f', 2));
            azimuth = gc->calculateAzimuth(settings->Latitude, settings->Longitude, latlon.latitude, latlon.longitude);
            rev_azimuth = gc->reverseAzimuth(azimuth);
            gc->setAzimuthText(tr("Азимут: %1/%2 град").arg(azimuth, 0, 'f', 0).arg(rev_azimuth, 0, 'f', 0));
            distance = gc->haversineDistance(settings->Latitude, settings->Longitude, latlon.latitude, latlon.longitude);
            gc->setDistanceText(tr("Расстояние: %1 км.").arg(distance, 0, 'f', 0));
        }
        qInfo() << "QTH Coordinates: " << "Callsign: " << callsign <<  "Lattitude: " << QString::number(latlon.latitude) << " longitude: " << latlon.longitude;
    } else {
        osm->clearMarkers();
        osm->showQSOMap(userData.callsign, settings->Latitude, settings->Longitude);
        if(gc->globe->isVisible()) {
            gc->clearMarkers();
            gc->globe->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
        }
        qDebug() << "Locator is not found.";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionGlobe_triggered()
{
    gc->setWindowTitle("Планета Земля");
    gc->show();

    // Отложим инициализацию, пока initializeGL() не вызовется
    QTimer::singleShot(0, this, [this]() {
        if(gc->globe->isVisible()) {
            gc->globe->resize(400,400);
            gc->globe->loadCountries(":/resources/countries.geo.json"); // загружаем страны (GeoJSON)
            gc->clearMarkers();
            gc->globe->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
            gc->globe->centerOnBalloons();
        }
    });
    ShowQSOLocation(qsoPanel->getCallsign(), qsoPanel->getQTHLocator());
}
//------------------------------------------------------------------------------------------------------------------------------------------

QVector<CountryEntry> MainWindow::loadCtyDatabase(const QString &fileName)
{
    QVector<CountryEntry> result;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return result;

    QTextStream ts(&file);

    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (!line.contains(':')) continue;

        QStringList parts = line.split(':');
        if (parts.size() < 8) continue;

        CountryEntry e;
        e.country    = parts[0].trimmed();
        e.cqZone     = parts[1].trimmed().toInt();
        e.ituZone    = parts[2].trimmed().toInt();
        e.continent  = parts[3].trimmed();
        e.latitude   = parts[4].trimmed().toDouble();
        e.longitude  = parts[5].trimmed().toDouble();
        e.utcOffset  = parts[6].trimmed().toDouble();
        e.mainPrefix = parts[7].trimmed();

        // собираем все строки с префиксами до ;
        QString prefixPart;
        if (parts.size() > 8)
            prefixPart = parts.mid(8).join(":").trimmed();

        while (!prefixPart.endsWith(';') && !ts.atEnd()) {
            QString nextLine = ts.readLine().trimmed();
            prefixPart += nextLine;
        }

        prefixPart.remove(';');
        QStringList rawList = prefixPart.split(',', Qt::SkipEmptyParts);

        for (QString p : rawList) {
            p = p.trimmed();
            // удаляем скобки и квадратные скобки
            QRegExp rx("\\(.*?\\)|\\[.*?\\]");
            p.replace(rx, "");
            p = p.trimmed();
            if (!p.isEmpty())
                e.prefixes.append(p);
        }
        result.push_back(e);
    }
    return result;
}
//------------------------------------------------------------------------------------------------------------------------------------------

CountryEntry MainWindow::findCountryByCall(const QString &call, const QVector<CountryEntry> &cty)
{
    QString upperCall = call.toUpper();
    CountryEntry best;
    int bestLen = -1;

    for (const auto &c : cty) {
        for (const QString &p : c.prefixes) {
            QString up = p.toUpper();
            if (upperCall.startsWith(up)) {
                if (up.length() > bestLen) {
                    best = c;
                    bestLen = up.length();
                }
            }
        }
    }
    return best;
}
//------------------------------------------------------------------------------------------------------------------------------------------

















