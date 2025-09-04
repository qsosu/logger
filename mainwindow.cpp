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
    qDebug() << "Support SSL: " << QSslSocket::supportsSsl() << " SSL Build Library: " << QSslSocket::sslLibraryBuildVersionString() << " SSL Library Version: " << QSslSocket::sslLibraryVersionString();

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

    qsoPanel = new QSOPanel(this, settings);
    ui->verticalLayout->insertWidget(0, qsoPanel);
    qsoPanel->dock(); // при запуске сразу в док
    ui->actionSync->setEnabled(false);

    qApp->setStyle("Fusion");
    qApp->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    setLanguage();
//------------------------------------------------------------------------------------------------------------------------------------------

    //Загрузка XML-файла с диапазонами и модуляциями
    if (LoadHamDefsSync()) {
        qDebug() << "HamDefs.xml прочитан.";
    } else {
        QMessageBox::critical(0, tr("Ошибка"), tr("Файл HamDefs.xml не найден. Завершаю работу."), QMessageBox::Ok);
        qDebug() << "Не удалось загрузить или открыть HamDefs.xml";
        close();
    }

    //Загрузка XML-файла с префиксами
    entries = loadPrefixDatabase();
 //------------------------------------------------------------------------------------------------------------------------------------------

    qDebug() << "Инициализация БД.";
    InitDatabase("data.db");

    //Настройка интерфейса
    ui->prevQSOtableView->setVisible(false);
    qsoPanel->setFrequenceText(settings->lastFrequence);

    // Кнопка-колокольчик
    bellBtn = new QPushButton(this);
    bellBtn->setIcon(QIcon(":/resources/images/bell.png"));
    bellBtn->setIconSize(QSize(16, 16));
    bellBtn->setFlat(true);
    bellBtn->setToolTip("Нет новых уведомлений");

    connect(bellBtn, &QPushButton::clicked, this, &MainWindow::openNewsWindow);
    statusBar()->insertPermanentWidget(0, bellBtn, 0);

    // Кнопка-конвертик
    notificationBtn = new QPushButton(this);
    notificationBtn->setIcon(QIcon(":/resources/images/notification.png"));
    notificationBtn->setIconSize(QSize(16, 16));
    notificationBtn->setFlat(true);
    notificationBtn->setToolTip("Есть новые сообщения в чате");

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

    magStormLabel->setText("Уровень ГМА: ");
    udpserverLbl->setText("UDP: ");
    flrigLbl->setText("FLRIG: ");
    qsosuLbl->setText("Сервис QSO.SU: ");
    qsosuLabel->setText("Offline");
    qsosuLabel->setStyleSheet("QLabel { color: red }");
    catLbl->setText("CAT: ");
    magStormLabel->setText("Уровень ГМА: Нет данных");

//------------------------------------------------------------------------------------------------------------------------------------------

    udpServer = new UdpServer(this);
    if (settings->udpServerEnable) {
        if (udpServer->start(settings->udpServerPort)) {
            connect(udpServer, &UdpServer::heartbeat, this, [=]() {
                ui->statusbar->showMessage(QString(tr("UDP: получен HEARTBEAT - %1 %2")).arg(QString::fromUtf8(udpServer->version), QString::fromUtf8(udpServer->revision)), 1000);
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
        flrigLabel->setText("Отключен");
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
    });
//------------------------------------------------------------------------------------------------------------------------------------------

    qrz = new QrzruCallbook(settings->QrzruLogin, settings->QrzruPassword);
    connect(qrz, &QrzruCallbook::error404, this, [=]() {
        ui->statusbar->showMessage(tr("QRZ API - данные не найдены"), 2000);
    });
    connect(qrz, &QrzruCallbook::error, this, [=]() {
        ui->statusbar->showMessage(tr("QRZ API - ошибка запроса"), 2000);
    });

    connect(qsoPanel, &QSOPanel::findCallTimer, this, [=]() {
        if (settings->enableQrzruCallbook || settings->useCallbook)
            FindCallData();
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

    osm = new Geolocation();
    osm->qth_lat = settings->Latitude;
    osm->qth_lng = settings->Longitude;
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
        about->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
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
    connect(api, SIGNAL(userDataUpdated()), this, SLOT(setUserData()));

    InitRecordsTable();
    InitPreviosQSOModel();

//------------------------------------------------------------------------------------------------------------------------------------------
    CAT = new cat_Interface(settings->catEnable);

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
    getCallsigns();
    fillDefaultFreq();

    if (settings->darkTheime) darkTheme();
    else lightTheme();

    catLabel->setText(tr("Отключен"));
    catLabel->setStyleSheet("QLabel { color: red; }");

//------------------------------------------------------------------------------------------------------------------------------------------
    update_prefix = new UpdateLogPrefix(db, entries);
    connect(update_prefix, SIGNAL(db_updated()), this, SLOT(RefreshRecords()));
//------------------------------------------------------------------------------------------------------------------------------------------
    reportSunChart = new ReportSunChart(db);
    connect(api, SIGNAL(MagStormUpdated(QJsonArray)), reportSunChart, SLOT(InsertMeasurement(QJsonArray)));
    connect(api, SIGNAL(MagStormUpdated(QJsonArray)), this, SLOT(UpdateMeasurement(QJsonArray)));
//------------------------------------------------------------------------------------------------------------------------------------------
    if(settings->showMap) on_actionShowMap_triggered();
    if(api->serviceAvailable) RemoveDeferredQSOs(); //Удаляем с QSO.SU ранее не удаленные QSO
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
    spotViewer = new SpotViewer(db, bandList, modeList, entries, CAT);
    connect(spotViewer, SIGNAL(setNewQSO(QString, QString, double, QString)), this, SLOT(setSpotQSO(QString, QString, double, QString)));
//------------------------------------------------------------------------------------------------------------------------------------------
    chats = new ChatController(db, api);

    api->getListSpotServers();
    connect(api, &HttpApi::spotServersReceived, this, [=](const QList<ServerInfo> &servers){
        if (servers.isEmpty()) {
            qWarning() << "Список серверов пуст!";
            return;
        }
        qDebug() << "Доступные серверы:";
        // Ищем сервер с минимальным client
        const ServerInfo *best = &servers.first();
        for (const auto &s : servers) {
            if (s.client < best->client) {
                best = &s;
            }
            qDebug() << best->host << best->port << "client =" << best->client;
        }

        tclient = new TelnetClient(db, best->host, best->port, this);
        connect(tclient, SIGNAL(newsMessageReceived()), this, SLOT(showBellIcon()));
        connect(tclient, SIGNAL(newChatReseived()), this, SLOT(showNotificationIcon()));
        connect(tclient, &TelnetClient::chatMessageReceived, chats, &ChatController::onMessageReceived);
        connect(tclient, SIGNAL(newSpotReseived()), spotViewer, SLOT(updateSpots()));
        qDebug() << "Подключаемся к серверу:" << best->host << best->port << "client =" << best->client;
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
    if(reportSunChart->isVisible()) reportSunChart->close();
    if(qsoPanel->isVisible()) qsoPanel->close();

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
    qApp->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    ui->tableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    ui->prevQSOtableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
    osm->qth_lat = settings->Latitude;
    osm->qth_lng = settings->Longitude;
}
//------------------------------------------------------------------------------------------------------------------------------------------
/* Database section */

void MainWindow::InitDatabase(QString dbFile) {

    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).exists())
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    database_file = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + dbFile;
    if (!CheckDatabase()) {
        qWarning() << "Database file does not exist. Creating new.";
        CreateDatabase();
    }
    if (ConnectDatabase()) {
        ui->statusbar->showMessage("Файл БД открыт", 3000);
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

void MainWindow::CreateDatabase() {
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
             "PRIMARY KEY(\"id\" AUTOINCREMENT))");
  qInfo() << "Creating UNIQUE INDEX";
  query.exec("CREATE UNIQUE INDEX IF NOT EXISTS idx_records_hash ON RECORDS(HASH) WHERE HASH IS NOT NULL AND HASH <> ''");
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
             "\"COMMENT\" TEXT)");
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
  qInfo() << "Creating triggers";
  query.exec("CREATE TRIGGER \"limit_spots\" "
             "AFTER INSERT ON \"spots\" "
             "BEGIN "
             "DELETE FROM \"spots\" "
             "WHERE \"id\" NOT IN ("
             "SELECT \"id\" FROM \"spots\" "
             "ORDER BY \"id\" DESC LIMIT 1000"
             "); "
             "END;");
  query.finish();
  db.close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::ConnectDatabase() {
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
        api->sendMessage(1, 102, "Тестовое сообщение");
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

void MainWindow::SetRecordsFilter(int log_id) {
    RecordsModel->setFilter(QString("callsign_id=%1").arg(log_id));
    RecordsModel->setSort(0, Qt::DescendingOrder); // Sort by ID
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RefreshRecords() {
    RecordsModel->select();
    while (RecordsModel->canFetchMore()) // FIX. Fetch more than 256 records!!!
        RecordsModel->fetchMore();
    ScrollRecordsToTop();
    countQSO->setText("Всего QSO: " + QString::number(RecordsModel->rowCount()));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::customMenuRequested(QPoint pos) {
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
        //
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
    QSqlRecord newRecord = RecordsModel->record();
    newRecord.remove(newRecord.indexOf("id"));
    newRecord.setValue("callsign_id", userData.callsign_id);
    newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
    newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
    newRecord.setValue("STATION_CALLSIGN", userData.callsign);
    newRecord.setValue("OPERATOR", userData.oper);

    QStringList data;
    data.append(api->callsignInfo);
    if(data.count() > 11) {
        newRecord.setValue("COUNTRY", data.at(8));
        newRecord.setValue("CONT", data.at(9));
        newRecord.setValue("ITUZ", data.at(10));
        newRecord.setValue("CQZ", data.at(11));
    }

    QString call = qsoPanel->getCallsign();
    newRecord.setValue("CALL", call);

    PrefixEntry* result = findPrefixEntry(entries, call);
    if (result) {
        newRecord.setValue("COUNTRY", result->country);
        newRecord.setValue("COUNTRY_CODE", result->country_code);
        newRecord.setValue("CONT", result->continent);
        newRecord.setValue("ITUZ", result->ituzone);
        newRecord.setValue("CQZ", result->cqzone);
        delete result;
    } else {
        qDebug() << "Страна не найдена для позывного" << call;
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
    newRecord.setValue("MODE", mode);
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

    if (RecordsModel->insertRecord(-1, newRecord)) {
        RecordsModel->submitAll();

        // Здесь надо отправлять QSO на сервис через API
        int LastID = RecordsModel->query().lastInsertId().toInt();
        QVariantList data;
        data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare << my_gridsquare << my_cnty;
        if(api->serviceAvailable) {
            api->SendQso(data);
            data << userData.callsign << userData.oper;
            logradio->SendQso(data);
        }
        RefreshRecords();
        ScrollRecordsToTop();
        qsoPanel->clearQSO();
        //ui->CallInput->setFocus();
        qsoPanel->SaveCallsignState();
    } else {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не возможно сохранить QSO. Ошибка базы данных!"), QMessageBox::Ok);
        qDebug() << "Ошибка: " << RecordsModel->lastError();
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RemoveQSOs(QModelIndexList indexes) {
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
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ScrollRecordsToBottom() {
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->selectRow(RecordsModel->rowCount() - 1);
    ui->tableView->scrollToBottom();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ScrollRecordsToTop() {
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->selectRow(0);
    ui->tableView->scrollToTop();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onCallsignsUpdated() {
  getCallsigns();
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

    if(qsoPanel->getStationCallsign() != "")
    {
        Coordinates latlon = osm->locatorToCoordinates(userData.gridsquare);
        osm->clearMarkers();
        osm->setQTHLocation(userData.callsign, latlon.latitude, latlon.longitude);
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
  qDebug() << "Creating database record for UDP message";
  QSqlRecord newRecord = RecordsModel->record();
  newRecord.remove(newRecord.indexOf("id"));
  newRecord.setValue("callsign_id", userData.callsign_id);
  newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
  newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
  newRecord.setValue("STATION_CALLSIGN", userData.callsign);
  newRecord.setValue("OPERATOR", userData.oper);
  newRecord.setValue("MY_GRIDSQUARE", userData.gridsquare);
  newRecord.setValue("MY_CNTY", userData.cnty);
  newRecord.setValue("QSO_DATE", udpServer->time_off.date().toString("yyyyMMdd"));
  newRecord.setValue("TIME_OFF", udpServer->time_off.time().toString("hhmm") + "00");
  newRecord.setValue("TIME_ON", udpServer->time_on.time().toString("hhmm") + "00");
  newRecord.setValue("CALL", QString::fromUtf8(udpServer->dx_call));
  newRecord.setValue("BAND", Helpers::GetBandByFreqHz(udpServer->tx_frequency_hz));
  newRecord.setValue("FREQ", udpServer->tx_frequency_hz);
  newRecord.setValue("MODE", QString::fromUtf8(udpServer->mode));
  newRecord.setValue("NAME", udpServer->call_name);
  newRecord.setValue("QTH", udpServer->call_qth);
  newRecord.setValue("GRIDSQUARE", QString::fromUtf8(udpServer->dx_grid));
  newRecord.setValue("RST_SENT", QString::fromUtf8(udpServer->report_sent));
  newRecord.setValue("RST_RCVD", QString::fromUtf8(udpServer->report_received));
  newRecord.setValue("COMMENT", QString::fromUtf8(udpServer->comments));
  newRecord.setValue("sync_state", 0);

  PrefixEntry* result = findPrefixEntry(entries, udpServer->dx_call);
  if (result) {
      newRecord.setValue("COUNTRY", result->country);
      newRecord.setValue("COUNTRY_CODE", result->country_code);
      newRecord.setValue("CONT", result->continent);
      newRecord.setValue("ITUZ", result->ituzone);
      newRecord.setValue("CQZ", result->cqzone);
      delete result;
  } else {
      qDebug() << "Страна не найдена для позывного" << udpServer->dx_call;
  }

  if(RecordsModel->insertRecord(-1, newRecord)) {
      if (RecordsModel->submitAll()) {
        qDebug() << "New record submited to database";

        int LastID = RecordsModel->query().lastInsertId().toInt();
        QVariantList data;
        QString datetime = udpServer->time_on.date().toString("yyyy-MM-dd") + "T" + udpServer->time_on.time().toString("hh:mm:00");
        data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << QString::fromUtf8(udpServer->dx_call) << Helpers::GetBandByFreqHz(udpServer->tx_frequency_hz) << QString::fromUtf8(udpServer->mode);
        data << udpServer->tx_frequency_hz << datetime << QString::fromUtf8(udpServer->name) << QString::fromUtf8(udpServer->report_sent) << QString::fromUtf8(udpServer->report_received) << "" << "" << QString::fromUtf8(udpServer->dx_grid);
        if(api->serviceAvailable) {
            api->SendQso(data);
            data << userData.gridsquare << userData.cnty << userData.callsign << userData.oper;
            logradio->SendQso(data);
        }

        if(udpServer->dx_grid.length() >= 3)
        {
            Coordinates latlon = osm->locatorToCoordinates(udpServer->dx_grid);
            osm->showQSOMap(udpServer->dx_call, latlon.latitude, latlon.longitude);
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
  qDebug() << "Creating database record for UDP ADIF message";
  QSqlRecord newRecord = RecordsModel->record();
  newRecord.remove(newRecord.indexOf("id"));
  newRecord.setValue("callsign_id", userData.callsign_id);
  newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
  newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
  newRecord.setValue("STATION_CALLSIGN", userData.callsign);
  newRecord.setValue("OPERATOR", userData.oper);
  newRecord.setValue("MY_GRIDSQUARE", userData.gridsquare);
  newRecord.setValue("MY_CNTY", userData.cnty);

  QDate qso_date = QDate::fromString(udpServer->adifData.value("QSO_DATE"), "yyyyMMdd");
  newRecord.setValue("QSO_DATE", udpServer->adifData.value("QSO_DATE"));
  newRecord.setValue("TIME_OFF", udpServer->adifData.value("TIME_OFF"));
  QTime time_on = QTime::fromString(udpServer->adifData.value("TIME_ON"), "hhmmss");
  newRecord.setValue("TIME_ON", udpServer->adifData.value("TIME_ON"));
  newRecord.setValue("CALL", udpServer->adifData.value("CALL"));
  newRecord.setValue("BAND", udpServer->adifData.value("BAND"));

  unsigned long long freqHz = static_cast<unsigned long long>(udpServer->adifData.value("FREQ").toDouble() * 1000000);
  newRecord.setValue("FREQ", freqHz);
  newRecord.setValue("MODE", udpServer->adifData.value("MODE"));

  QEventLoop loop;
  if(api->serviceAvailable) {
      if(settings->useCallbook) {
          api->getCallbook(udpServer->adifData.value("CALL"));
          QObject::connect(api, &HttpApi::userDataUpdated, &loop, &QEventLoop::quit);
          loop.exec();
      }
  }
  newRecord.setValue("NAME", udpServer->call_name);
  newRecord.setValue("QTH", udpServer->call_qth);
  newRecord.setValue("GRIDSQUARE", udpServer->adifData.value("GRIDSQUARE"));
  newRecord.setValue("RST_SENT", udpServer->adifData.value("RST_SENT"));
  newRecord.setValue("RST_RCVD", udpServer->adifData.value("RST_RCVD"));
  newRecord.setValue("COMMENT", udpServer->adifData.value("COMMENT"));
  newRecord.setValue("sync_state", 0);

  PrefixEntry* result = findPrefixEntry(entries, udpServer->adifData.value("CALL"));
  if (result) {
      newRecord.setValue("COUNTRY", result->country);
      newRecord.setValue("COUNTRY_CODE", result->country_code);
      newRecord.setValue("CONT", result->continent);
      newRecord.setValue("ITUZ", result->ituzone);
      newRecord.setValue("CQZ", result->cqzone);
      delete result;
  } else {
      qDebug() << "Страна не найдена для позывного" << udpServer->adifData.value("CALL");
  }

  if(RecordsModel->insertRecord(-1, newRecord)) {
      if (RecordsModel->submitAll()) {
        qDebug() << "New record submited to database";

        int LastID = RecordsModel->query().lastInsertId().toInt();
        QVariantList data;
        QString datetime = qso_date.toString("yyyy-MM-dd") + "T" + time_on.toString("hh:mm:00");
        data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << udpServer->adifData.value("CALL") << udpServer->adifData.value("BAND") << udpServer->adifData.value("MODE") << freqHz;
        data << datetime << "" << udpServer->adifData.value("RST_SENT") << udpServer->adifData.value("RST_RCVD") << "" << "" << udpServer->adifData.value("GRIDSQUARE");

        if(api->serviceAvailable) {
            api->SendQso(data);
            qDebug() << "SendQso " << data;
            data << userData.gridsquare << userData.cnty << userData.callsign << userData.oper;
            logradio->SendQso(data);
         }

        if(udpServer->adifData.value("GRIDSQUARE").length() >= 3)
        {
            Coordinates latlon = osm->locatorToCoordinates(udpServer->adifData.value("GRIDSQUARE"));
            osm->showQSOMap(udpServer->adifData.value("CALL"), latlon.latitude, latlon.longitude);
        }
        RefreshRecords();
        ScrollRecordsToTop();
        qsoPanel->clearQSO();
      }
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::FindCallData() {
    QString callsign = qsoPanel->getCallsign();

    if (callsign.isEmpty() && callsign.count() > 3) {
        qDebug() << "Need find callbook: " << callsign;
        qsoPanel->ClearCallbookFields();
        qsoPanel->setQSOSUserVisible(false);
        qsoPanel->setQSOSUserLabelVisible(false);
        qsoPanel->setUserSRRVisible(false);
        qsoPanel->setUserSRRLabelVisible(false);
        osm->clearMarkers();
        osm->setQTHLocation(userData.callsign, settings->Latitude, settings->Longitude);
        return;
    }
    if(callsign.length() > 3)
    {
        Coordinates latlon = osm->locatorToCoordinates(userData.gridsquare);
        osm->clearMarkers();
        osm->setQTHLocation(userData.callsign, latlon.latitude, latlon.longitude);

        if(api->serviceAvailable && settings->useCallbook) {
            api->getCallbook(callsign);
            qDebug() << "Use QSO.SU callbook.";
        }
        if(settings->enableQrzruCallbook){
            QStringList data = qrz->Get(callsign);
            qsoPanel->setName((data.at(0).length() > 0) ? data.at(0) : "");
            qsoPanel->setQTH((data.at(1).length() > 0) ? data.at(1) : "");
            qsoPanel->setQTHLocator((data.at(2).length() > 0) ? data.at(2).toUpper() : "");
            qsoPanel->setRDA((data.at(3).length() > 0) ? data.at(3).toUpper() : "");
            qDebug() << "Use QRZ.RU callbook.";
        }
    }
    showPreviosQSO(callsign);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setUserData() {
  QStringList data;
  double lat, lng;

  data.append(api->callsignInfo);
  qsoPanel->setName((data.at(0).length() > 0) ? data.at(0) : "");
  qsoPanel->setQTH((data.at(1).length() > 0) ? data.at(1) : "");

  udpServer->call_name = qsoPanel->getName();
  udpServer->call_qth = qsoPanel->getQTH();

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

  if(qsoPanel->getQTHLocator().length() > 3)
  {
      lat = data.at(13).toDouble();
      lng = data.at(14).toDouble();
      osm->showQSOMap(qsoPanel->getCallsign(), lat, lng);
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::fillDefaultFreq()
{
  double freqMhz = BandToDefaultFreq(qsoPanel->getBand());
  qsoPanel->setFrequence(freqMhz);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SyncQSOs(QModelIndexList indexes) {
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
            data << userData.callsign << userData.oper;
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
    qDebug() << "Загрузка файла HamDefs.xml с сервера QSO.SU";

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
        qDebug() << "Открытие файла HamDefs.xml с диска.";
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

    qDebug() << QString::number(idx) << hash;

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

    qDebug() << "Remove Deferred QSOs.";
    db.transaction();
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM DELRECORDS");
    query.exec(); query.next();
    rowcount = query.value(0).toInt();

    query.prepare("SELECT HASH FROM DELRECORDS");
    query.exec(); query.next();

    for(int i = rowcount; i > 0; i--) {
        api->deleteByHashLog(query.value(0).toString());
        qDebug() << "Del QSO: " << query.value(0).toString();
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

void MainWindow::setFreq(long freq)
{
    settings->lastFrequence = QString::number((double) freq / 1000000, 'f', 6);
    qsoPanel->setFrequenceText(settings->lastFrequence);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setBand(int band)
{
    settings->lastBand = bandList[band].band_name;
    qsoPanel->setBand(bandList[band].band_name);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setMode(int mode)
{
    settings->lastMode = modeList[mode].mode_name;
    qsoPanel->setMode(modeList[mode].mode_name);
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
            qDebug() << "id: " << count << "Call: " << call << " Gridsquare: " << gridsquare << " Lat: " << QString::number(latlon.latitude) << " Lng: " << QString::number(latlon.longitude);
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
    QString filterString = QString("CALL = '%1'").arg(call);
    PrevRecordsModel->setFilter(filterString);
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
    if(settings->language == "Русский") {
        qtLanguageTranslator.load("://translations/ru_Ru.qm");
        qApp->installTranslator(&qtLanguageTranslator);
    }

    if(settings->language == "English") {
        QString path = QCoreApplication::applicationDirPath() + "/translations";
        QDir dir(path);
        QStringList filters;
        filters << "en_US.qm";

        QStringList results = dir.entryList(filters, QDir::Files);
        if(!results.isEmpty()) {
            qDebug() << "Файл перевода найден:" << dir.filePath(results.first());
        } else {
            qDebug() << "Файл перевода не найден.";
        }
        qtLanguageTranslator.load(dir.filePath(results.first()));
        qApp->installTranslator(&qtLanguageTranslator);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionUploadQSOs_triggered()
{
    uploadLogs = new UploadingLogs(db, settings, entries);
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
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
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

QList<PrefixEntry> MainWindow::loadPrefixDatabase()
{
    QList<PrefixEntry> entries;
    QString basePath;

    #ifdef Q_OS_MAC
        // В macOS: внутри .app → подняться на 3 уровня
        basePath = QCoreApplication::applicationDirPath() + "/Prefixes.xml";
    #elif defined(Q_OS_WIN) || defined(Q_OS_LINUX)
        // В Windows и Linux файл лежит рядом с .exe
        basePath = QCoreApplication::applicationDirPath() + "/Prefixes.xml";
    #else
        // fallback
        basePath = "Prefixes.xml";
    #endif

    QFile file(basePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open XML file:" << basePath;
        return entries;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "PrefixEntry") {
            auto attrs = xml.attributes();
            PrefixEntry entry;
            entry.country = attrs.value("Country").toString();
            entry.country_code = attrs.value("Country_code").toString();
            entry.continent = attrs.value("Continent").toString();
            entry.cqzone = attrs.value("CQZone").toString();
            entry.ituzone = attrs.value("ITUZone").toString();
            entry.dxcc = attrs.value("DXCC").toString();
            entry.latitude = attrs.value("Latitude").toString();
            entry.longitude = attrs.value("Longitude").toString();
            QString regexString = attrs.value("PrefixList").toString();
            entry.regexList = regexString.split("|", Qt::SkipEmptyParts);
            entries.append(entry);
        }
    }
    file.close();
    return entries;
}
//------------------------------------------------------------------------------------------------------------------------------------------

PrefixEntry* MainWindow::findPrefixEntry(const QList<PrefixEntry>& entries, const QString& callsign)
{
    QString csUpper = callsign.toUpper();

    for (const auto& entry : entries) {
        for (const auto& rawPattern : entry.regexList) {
            QString pattern = rawPattern;

            // Обработка экранирования: \Z => $ (конец строки)
            pattern.replace("\\Z", "$");

            // Явно указываем, что сравнение с начала строки
            if (!pattern.startsWith("^")) {
                pattern = "^" + pattern;
            }
            QRegularExpression re(pattern);
            if (!re.isValid()) {
                qDebug() << "Invalid regex:" << pattern;
                continue;
            }
            if (re.match(csUpper).hasMatch()) {
                return new PrefixEntry(entry);
            }
        }
    }
    return nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionUpdatePrefix_triggered()
{
    update_prefix->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionCheckCountry_triggered()
{
    bool ok;
    QString Call = QInputDialog::getText(this, "Введите территории по позывному:", "Введите позывной:", QLineEdit::Normal, "", &ok);
    PrefixEntry* result = findPrefixEntry(entries, Call);
    if (result) {
        QMessageBox::information(this, "Проверка территории по позывному",
                                 "Позывной: " + Call + "\n" +
                                 "Страна: " + result->country + "\n" +
                                 "Код страны: " + result->country_code + "\n" +
                                 "Континент: " + result->continent + "\n" +
                                 "CQZ: " + result->cqzone + "\n" +
                                 "ITUZ :" + result->ituzone);
        delete result;
    } else {
        QMessageBox::information(this, "Проверка территории по позывному", "Страна не найдена для позывного:" + Call);
        qDebug() << "Страна не найдена для позывного:" << Call;
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
    QString lastTimeStr;
    QDateTime lastTime;

    QSqlQuery query(db);
    QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();

    // Проверяем количество записей
    int rowCount = 0;
    if (!query.exec("SELECT COUNT(*) FROM magnetic_storm")) {
        qDebug() << "Ошибка подсчета строк таблицы magnetic_storm:" << query.lastError().text();
    } else if (query.next()) {
        rowCount = query.value(0).toInt();
    }

    if (rowCount == 0) {
        // Нет записей — берем за 365 дней
        lastTime = currentTimeUtc.addDays(-365);
        lastTime.setTimeSpec(Qt::UTC);
        lastTimeStr = lastTime.toString(Qt::ISODate);
        qDebug() << "Данных нет. Загружаю за 365 дней: " << lastTimeStr;
    }
    else if (rowCount < 100) {
        // Мало записей — тоже берем 365 дней назад
        lastTime = currentTimeUtc.addDays(-365);
        lastTime.setTimeSpec(Qt::UTC);
        lastTimeStr = lastTime.toString(Qt::ISODate);
        qDebug() << "Записей меньше 100 (" << rowCount << "). Загружаю остальные: " << lastTimeStr;
    }
    else {
        // Получаем последнее время измерения
        if (!query.exec("SELECT measurement_time FROM magnetic_storm ORDER BY measurement_time DESC LIMIT 1")) {
            qDebug() << "Ошибка запроса:" << query.lastError().text();
        }

        if (query.next()) {
            lastTimeStr = query.value(0).toString();
            lastTime = QDateTime::fromString(lastTimeStr, Qt::ISODate);
            lastTime.setTimeSpec(Qt::UTC);
            qDebug() << "Последнее время измерения: " << lastTimeStr;

            qint64 diffSecs = lastTime.secsTo(currentTimeUtc);
            if (diffSecs < 3 * 3600) {
                qDebug() << "Разница менее 3 часов. Загрузка с сервера не требуется.";
                reportSunChart->showMagneticStormChart();
                return;
            }
        }
        reportSunChart->showMagneticStormChart();
    }
    // Загружаем данные начиная с lastTime
    api->getMagneticStormInfo(lastTime.toString("yyyy-MM-dd"));
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
        QString currentTimeStr;
        QDateTime currentTimeUtc = QDateTime::currentDateTimeUtc();
        currentTimeUtc.setTimeSpec(Qt::UTC);
        currentTimeStr = currentTimeUtc.toString(Qt::ISODate);
        api->getMagneticStormInfo(currentTimeUtc.toString("yyyy-MM-dd"));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::UpdateMeasurement(QJsonArray data)
{
    if (!data.isEmpty()) {
        QJsonObject lastObject = data.first().toObject();
        int level = lastObject["value"].toInt();
        magStormLabel->setText(QString("Уровень ГМА: %1").arg(level));
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

void MainWindow::showBellIcon()
{
    bellBtn->setIcon(QIcon(":/resources/images/bell_new.png"));
    bellBtn->setToolTip("Есть новые сообщения");
    hasNewNews = true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::showNotificationIcon()
{
    notificationBtn->setIcon(QIcon(":/resources/images/notification_new.png"));
    notificationBtn->setToolTip("Есть новые сообщения в чате");
    hasNewMessages = true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::openNewsWindow()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Лента новостей");
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
        bellBtn->setToolTip("Нет новых уведомлений");
        hasNewNews = false;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_actionShowSpots_triggered()
{
    spotViewer->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::saveHeaderState(QTableView *tableView) {
    QSettings settings("QSO.SU", "QSOLogger");
    settings.setValue("headerStateMain", tableView->horizontalHeader()->saveState());
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::restoreHeaderState(QTableView *tableView) {
    QSettings settings("QSO.SU", "QSOLogger");
    QByteArray state = settings.value("headerStateMain").toByteArray();
    if (!state.isEmpty()) {
        tableView->horizontalHeader()->restoreState(state);
    }
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::setSpotQSO(QString call, QString band, double freq, QString mode)
{
    qsoPanel->setCallsign(call);
    qsoPanel->setBand(band);
    qsoPanel->setFrequenceText(QString::number(freq / 1000000.0, 'f', 6));
    qsoPanel->setMode(mode);
}
//-----------------------------------------------------------------------------------------------------

void MainWindow::on_actionImportADIF_triggered()
{
    imp_adif = new ImportADIF(db, entries);
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
    chats->show();

    // После просмотра сбрасываем иконку
    if (hasNewMessages) {
        notificationBtn->setIcon(QIcon(":/resources/images/notification.png"));
        notificationBtn->setToolTip("Нет новых сообщений в чате");
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




