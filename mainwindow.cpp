#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "delegations.h"
#include "QRegularExpressionValidator"
#include "qlistview.h"
#include <QCompleter>

#define SET_ROBOT_FONT
#define VERSION "1.8.1"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setWindowTitle("QSO Logger v." + QString::fromStdString(VERSION));

  QFontDatabase::addApplicationFont("://resources/fonts/Roboto-Regular.ttf");

  ui->callInput->setStyleSheet("color: black; font-weight: bold");
  //BugFix Только латинские символы и цифры
  ui->callInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9/]*$"), this));

  //BugFix Только цифры
  ui->rstrInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[+-]?[0-9]*$"), this));
  ui->rstsInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[+-]?[0-9]*$"), this));
  ui->gridsquareInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^([a-zA-Z]{2})([0-9]{2})(((([a-zA-Z]{2}?)?)([0-9]{2}?)?)([a-zA-Z]{2}?)?)$/"), this));

  ui->bandCombo->blockSignals(true);
  ui->modeCombo->blockSignals(true);

  //ui->qthlocEdit->setReadOnly(true);
  //ui->rdaEdit->setReadOnly(true);
  ui->actionSync->setEnabled(false);

  EverySecondTimer = new QTimer(this);
  EverySecondTimer->setInterval(1000);
  connect(EverySecondTimer, SIGNAL(timeout()), this, SLOT(UpdateFormDateTime()));

  ui->showCurrentTime->setChecked(true);
  UpdateFormDateTime();
  EverySecondTimer->start();

  connect(ui->showCurrentTime, &QCheckBox::toggled, this, [=](bool checked) {
      if (checked) {
          UpdateFormDateTime();
          EverySecondTimer->start();
      } else {
          EverySecondTimer->stop();
          //BugFix ui->timeInput->clear();
      }
  });

  //ui->modeCombo->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  //ui->modeCombo->setMaxVisibleItems(130);

  //Проверка использования и версий SSL
  //qDebug() << "Support SSL: " << QSslSocket::supportsSsl() << " SSL Build Library: " << QSslSocket::sslLibraryBuildVersionString() << " SSL Library Version: " << QSslSocket::sslLibraryVersionString();

  settings = new Settings();
  settings->setAttribute(Qt::WA_QuitOnClose, false);
  connect(settings, SIGNAL(SettingsChanged()), this, SLOT(onSettingsChanged()));
  connect(ui->actionSettings, &QAction::triggered, this, [=]() {
      settings->show();
  });


  //--------Logger CAT
  loggercat = new loggerCAT();
  loggercat->setAttribute(Qt::WA_QuitOnClose, false);
  connect(loggercat, SIGNAL(CATSettingsChanged()), this, SLOT(onSettingsChanged()));
  connect(ui->actionLogger_CAT, &QAction::triggered, this, [=]() {
      loggercat->show();
  });
  connect(loggercat, &loggerCAT::rigupd, this, [=]() {
      //if(loggercat->useLoggerCAT->isChecked()){
      QList <QString> info = loggercat->rigRequesting();
      unsigned int rigFreq = info.at(0).toUInt();
      QString rigMode = info.at(1);
      ui->bandCombo->setCurrentText(Helpers::GetBandByFreqHz(rigFreq));
      ui->freqInput->setText(QString::number((double) rigFreq / 1000000, 'f', 6));
      ui->modeCombo->setCurrentText(rigMode);
      //}
  });

  ui->modeCombo->setEditable(true); //Включаем встроенный QLineEdit
  ui->modeCombo->setInsertPolicy(QComboBox::NoInsert); // отключаем вставку новых элементов из QLineEdit
  ui->modeCombo->completer()->setCompletionMode(QCompleter::CompletionMode::PopupCompletion); // устанавливаем модель автодополнения (по умолчанию стоит InlineCompletition)
  ui->modeCombo->completer()->setModelSorting(QCompleter::UnsortedModel);

  qApp->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));

  InitDatabase("data.db");

  udpReceiver = new UdpReceiver(this);
  if (settings->udpServerEnable) {
      if (udpReceiver->start(settings->udpServerPort)) {
          connect(udpReceiver, &UdpReceiver::heartbeat, this, [=]() {
              ui->statusbar->showMessage(QString("UDP: получен HEARTBEAT - %1 %2").arg(QString::fromUtf8(udpReceiver->version), QString::fromUtf8(udpReceiver->revision)), 1000);
          });

          connect(udpReceiver, &UdpReceiver::logged, this, &MainWindow::onUdpLogged);
          ui->statusbar->showMessage("UDP сервер запущен на порту " + QString::number(settings->udpServerPort), 3000);
          ui->udpserverLabel->setText("Запущен");
          ui->udpserverLabel->setStyleSheet("QLabel { color: green }");
      } else {
          ui->udpserverLabel->setText("Ошибка");
          ui->udpserverLabel->setStyleSheet("QLabel { color: red }");
          //QMessageBox::critical(nullptr, "Ошибка", "Ошибка запуска UDP сервера");
      }
  }

  flrig = new Flrig(settings->flrigHost, settings->flrigPort, 500, this);
  connect(flrig, &Flrig::connected, this, [=]() {
    ui->flrigLabel->setText("Подключен");
    ui->flrigLabel->setStyleSheet("QLabel { color: green; }");
    ui->statusbar->showMessage("Подключен к FLRIG", 1000);
  });
  connect(flrig, &Flrig::disconnected, this, [=]() {
    ui->flrigLabel->setText("Отключен");
    ui->flrigLabel->setStyleSheet("");
    ui->statusbar->showMessage("Отключен от FLRIG", 1000);
  });
  connect(flrig, &Flrig::updated, this, [=]() {
    if (flrig->getConnState()) {
      ui->bandCombo->setCurrentText(Helpers::GetBandByFreqHz(flrig->getFrequencyHz()));
      ui->freqInput->setText(QString::number((double) flrig->getFrequencyHz() / 1000000, 'f', 6));
      ui->modeCombo->setCurrentText(flrig->getMode());
    }
  });
  connect(flrig, &Flrig::rpcError, this, [=]() {
    ui->statusbar->showMessage(QString("Ошибка XML-RPC: %1 %2").arg(QString::number(flrig->getErrorCode()), flrig->getErrorString()), 300);
  });



  adif = new Adif(db);
  connect(ui->actionExportAdif, &QAction::triggered, this, [=]() {
      if (userData.callsign_id > 0) adif->Export(userData.callsign_id);
  });

  qrz = new QrzruCallbook(settings->QrzruLogin, settings->QrzruPassword);
  connect(qrz, &QrzruCallbook::error404, this, [=]() {
      ui->statusbar->showMessage("QRZ API - данные не найдены", 2000);
  });
  connect(qrz, &QrzruCallbook::error, this, [=]() {
      ui->statusbar->showMessage("QRZ API - ошибка запроса", 2000);
  });

  CallTypeTimer = new QTimer(this);
  CallTypeTimer->setSingleShot(true);
  CallTypeTimer->setInterval(2000);
  connect(CallTypeTimer, &QTimer::timeout, this, [=]() {
      if (settings->enableQrzruCallbook) FindCallDataQrzru();
  });

  api = new HttpApi(db, settings->accessToken);
  connect(api, &HttpApi::emptyToken, this, [=]() {
      QMessageBox::critical(0, "Ошибка", "Не указан ACCESS TOKEN", QMessageBox::Ok);
      return;
  });
  connect(api, &HttpApi::error, this, [=](QNetworkReply::NetworkError error) {
      QMessageBox::critical(0, "Ошибка", "Ошибка обращения к API: " + QString::number(error), QMessageBox::Ok);
      return;
  });
  connect(api, &HttpApi::synced, this, &MainWindow::onQsoSynced);

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
  connect(ui->stationCallsignCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onStationCallsignChanged()));
  connect(ui->operatorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onOperatorChanged()));
  connect(ui->clearQsoButton, SIGNAL(clicked()), this, SLOT(ClearQso()));
  connect(ui->callInput, SIGNAL(textEdited(const QString&)), this, SLOT(CallsignToUppercase(const QString&)));
  connect(ui->saveQsoButton, SIGNAL(clicked()), this, SLOT(SaveQso()));
  connect(ui->refreshButton, &QPushButton::clicked, this, [=] {
      RefreshRecords();
      //ScrollRecordsToBottom();
      ScrollRecordsToTop();
  });
  connect(ui->bandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(fillDefaultFreq()));
  connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

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

  connect(api, SIGNAL(HamDefsUploaded()), this, SLOT(HamDefsUploaded()));
  connect(api, SIGNAL(HamDefsError()), this, SLOT(HamDefsError()));

  qInfo() << "QSOLogger v." << VERSION << " started.";
  LoadHamDefs(); //Загрузка XML-файла с диапазонами и модуляциями

  InitRecordsTable();
  getCallsigns();
  fillDefaultFreq();

  ui->stationCallsignCombo->blockSignals(true);

  if(settings->darkTheime) darkTheime();
  else qApp->setPalette(style()->standardPalette());
}
//------------------------------------------------------------------------------------------------------------------------------------------

MainWindow::~MainWindow() {
    if (loggercat->portOpened){ //закрываем порт, если он открыт
        loggercat->serialPort.close();
        loggercat->portOpened=false;
    }
  delete ui;
}

/* Database section */
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitDatabase(QString dbFile) {
    database_file = qApp->applicationDirPath() + "/" + dbFile;
    if (!CheckDatabase()) {
        qWarning() << "Database file does not exist. Creating new.";
        CreateDatabase();
    }
    if (ConnectDatabase()) {
        ui->statusbar->showMessage("Файл БД открыт", 3000);
    } else {
        qWarning() << "Error while open database file";
        QMessageBox::critical(0, "Ошибка", "Ошибка открытия файла БД", QMessageBox::Ok);
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
  query.exec("CREATE TABLE \"callsigns\" (\"id\" INTEGER NOT NULL, \"qsosu_id\" INTEGER NOT NULL DEFAULT 0, \"type\" INTEGER NOT NULL DEFAULT 10, \"name\" TEXT NOT NULL, \"validity_start\" INTEGER NOT NULL DEFAULT 0, \"validity_stop\" INTEGER NOT NULL DEFAULT 0, \"gridsquare\" TEXT NOT NULL, \"cnty\" TEXT, \"ituz\" INTEGER NOT NULL DEFAULT 0, \"cqz\" INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(\"id\" AUTOINCREMENT))");
  qInfo() << "Creating RECORDS table";
  query.exec("CREATE TABLE \"records\" ("
             "\"id\" INTEGER NOT NULL,"
             "\"callsign_id\" INTEGER NOT NULL,"
             "\"qsosu_callsign_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"qsosu_operator_id\" INTEGER NOT NULL DEFAULT 0,"
             "\"STATION_CALLSIGN\" TEXT NOT NULL,"
             "\"OPERATOR\" TEXT NOT NULL,"
             "\"MY_GRIDSQUARE\" TEXT NOT NULL,"
             "\"MY_CNTY\" TEXT,"
             "\"CALL\" TEXT NOT NULL,"
             "\"QSO_DATE\" TEXT NOT NULL,"
             "\"TIME_ON\" TEXT NOT NULL,"
             "\"TIME_OFF\" TEXT NOT NULL,"
             "\"BAND\" TEXT NOT NULL,"
             "\"FREQ\" INTEGER NOT NULL,"
             "\"MODE\" TEXT NOT NULL,"
             "\"RST_SENT\" TEXT NOT NULL,"
             "\"RST_RCVD\" TEXT NOT NULL,"
             "\"NAME\" TEXT,"
             "\"QTH\" TEXT,"
             "\"GRIDSQUARE\" TEXT,"
             "\"CNTY\" TEXT,"
             "\"COMMENT\" TEXT,"
             "\"sync_state\" INTEGER NOT NULL DEFAULT 0,"
             "PRIMARY KEY(\"id\" AUTOINCREMENT))");
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
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            SaveQso();
        }

        if (event->key() == Qt::Key_Escape) {
            ClearQso();
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::getCallsigns() {
  ui->stationCallsignCombo->clear();
  //ui->stationCallsignCombo->setCurrentIndex(0); //Bug Fix Change
  //ui->stationCallsignCombo->addItem("- Не выбран -", 0); //Bug Fix Change
  ui->operatorCombo->clear();
  //ui->operatorCombo->setCurrentIndex(0); //Bug Fix Change
  //ui->operatorCombo->addItem("- Не выбран -", 0); //Bug Fix Change

  QSqlQuery query(db);
  query.exec("SELECT id, qsosu_id, type, name, gridsquare, cnty FROM callsigns");
  while(query.next()) {
      int id = query.value(0).toInt();
      int qsosu_id = query.value(1).toInt();
      int type = query.value(2).toInt();
      QString name = query.value(3).toString();
      QString gridsquare = query.value(4).toString();
      QString cnty = query.value(5).toString();
      ui->stationCallsignCombo->addItem(name, QList<QVariant>() << id << qsosu_id << type << gridsquare << cnty);
      if(type == 0) ui->operatorCombo->addItem(name, QList<QVariant>() << id << qsosu_id); //Bug Fix Change
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::InitRecordsTable() {
  RecordsModel = new QSqlTableModel(this);
  RecordsModel->setTable("records");

  //RecordsModel->setEditStrategy(QAbstractItemView::NoEditTriggers);
  //RecordsModel->setQuery("SELECT * FROM RECORDS ORDER BY ID");
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
  RecordsModel->setHeaderData(21, Qt::Horizontal, tr("Коммент."));
  RecordsModel->setHeaderData(22, Qt::Horizontal, tr("Синхр."));

  ui->tableView->setModel(RecordsModel);
  ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableView->setColumnHidden(0, true);
  ui->tableView->setColumnHidden(1, true);
  ui->tableView->setColumnHidden(2, true);
  ui->tableView->setColumnHidden(3, true);
  ui->tableView->setColumnHidden(4, true);
  ui->tableView->setColumnHidden(5, true);
  ui->tableView->setColumnHidden(6, true);
  ui->tableView->setColumnHidden(7, true);

  ui->tableView->setItemDelegateForColumn(8, new FormatCallsign(ui->tableView));
  ui->tableView->setItemDelegateForColumn(9, new FormatDate(ui->tableView));
  ui->tableView->setItemDelegateForColumn(10, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(11, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(13, new FormatFreq(ui->tableView));
  ui->tableView->setItemDelegateForColumn(22, new FormatSyncState(ui->tableView));
  ui->tableView->horizontalHeader()->swapSections(0, 22); //Swap columns

  ui->tableView->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView *horizontalHeader = ui->tableView->horizontalHeader();
  horizontalHeader->setSectionResizeMode(QHeaderView::Interactive);
  horizontalHeader->setMinimumSectionSize(70);
  horizontalHeader->setStretchLastSection(true);
  horizontalHeader->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));

  QHeaderView *verticalHeader = ui->tableView->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setDefaultSectionSize(20);

  ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
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
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::customMenuRequested(QPoint pos) {
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if (indexes.count() == 0) return;

    QMenu *menu = new QMenu(this);
    QAction *deleteAction = new QAction((indexes.count() > 1) ? "Удалить выбранные" : "Удалить", this);
    connect(deleteAction, &QAction::triggered, this, [=]() {
        RemoveQSOs(indexes);
    });

    QAction *exportAction = new QAction((indexes.count() > 1) ? "Экспорт выбранных в ADIF" : "Экспорт в ADIF", this);
    connect(exportAction, &QAction::triggered, this, [=]() {
        QList<int> items;
        for (int i = indexes.count(); i > 0; i--) {
            int index = indexes.at(i-1).row();
            int id = RecordsModel->data(RecordsModel->index(index, 0)).toInt();
            items.append(id);
        }
        adif->ExportPartial(items);
    });

    QAction *syncAction = new QAction((indexes.count() > 1) ? "Синхронизировать выбранные" : "Синхронизировать", this);
    connect(syncAction, &QAction::triggered, this, [=]() {
        SyncQSOs(indexes);
    });

    menu->addAction(deleteAction);
    menu->addAction(exportAction);
    menu->addAction(syncAction);
    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SaveQso()
{
  //qDebug() << "USER DATA:" << userData.callsign_id << userData.qsosu_callsign_id << userData.qsosu_operator_id << userData.callsign << userData.oper << userData.gridsquare << userData.cnty;

  if(ui->callInput->text().size() < 3) {
       QMessageBox::critical(0, "Ошибка", "Не возможно сохранить QSO. Не введен позывной кореспондента!", QMessageBox::Ok);
       return;
  }

  QSqlRecord newRecord = RecordsModel->record();
  newRecord.remove(newRecord.indexOf("id"));
  newRecord.setValue("callsign_id", userData.callsign_id);
  newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
  newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
  newRecord.setValue("STATION_CALLSIGN", userData.callsign);
  newRecord.setValue("OPERATOR", userData.oper);
  QString call = ui->callInput->text();
  newRecord.setValue("CALL", call);

  //QDate qsoDate = QDate::fromString(ui->dateInput->text(), "yyyy-MM-dd");
  QDate qsoDate = ui->dateInput->date();
  newRecord.setValue("QSO_DATE", qsoDate.toString("yyyyMMdd"));
  //QTime qsoTime = QTime::fromString(ui->timeEdit->text(), "hh:mm:ss");
  QTime qsoTime = ui->timeEdit->time();

  //BugFix
  QString date = qsoDate.toString("yyyy-MM-dd");
  QString time = qsoTime.toString("hh:mm:ss");
  QString datetime = date + "T" + time;


  QString qsoTimeFormated = qsoTime.toString("hhmmss");
  newRecord.setValue("TIME_ON", qsoTimeFormated);
  newRecord.setValue("TIME_OFF", qsoTimeFormated);

  //QString band = ui->bandCombo->currentText();
  QString band = getBandValue(ui->bandCombo->currentIndex()); //BugFix
  newRecord.setValue("BAND", band);

  unsigned long long freqHz = static_cast<unsigned long long>(ui->freqInput->text().toDouble() * 1000000);
  newRecord.setValue("FREQ", freqHz);

  //QString mode = ui->modeCombo->currentText();
  QString mode = getModeValue(ui->modeCombo->currentText()); //BugFix
  newRecord.setValue("MODE", mode);

  QString rsts = ui->rstsInput->text();
  newRecord.setValue("RST_SENT", rsts);
  QString rstr = ui->rstrInput->text();
  newRecord.setValue("RST_RCVD", rsts);
  QString name = ui->nameInput->text();
  newRecord.setValue("NAME",name);
  QString qth = ui->qthInput->text();
  newRecord.setValue("QTH", qth);
  QString gridsquare = ui->gridsquareInput->text().toUpper();
  newRecord.setValue("GRIDSQUARE", gridsquare);
  QString cnty = ui->cntyInput->text().toUpper();
  newRecord.setValue("CNTY", cnty);
  newRecord.setValue("COMMENT", ui->commentInput->text());
  newRecord.setValue("sync_state", 0);

  //BugFix Added my_gridsquare and my_cnty
  QString my_gridsquare = ui->qthlocEdit->text().toUpper();
  newRecord.setValue("MY_GRIDSQUARE", my_gridsquare);

  QString my_cnty = ui->rdaEdit->text().toUpper();
  newRecord.setValue("MY_CNTY", my_cnty);

  if (RecordsModel->insertRecord(-1, newRecord)) {
      RecordsModel->submitAll();

      // Здесь надо отправлять QSO на сервис через API
      int LastID = RecordsModel->query().lastInsertId().toInt();
      QVariantList data;
      data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
      data << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare << my_cnty << my_gridsquare;
      api->SendQso(data);

      RefreshRecords();
      //ScrollRecordsToBottom();
      ScrollRecordsToTop();
      ClearQso();
      ui->callInput->setFocus();
      SaveCallsignState();
  } else {
      QMessageBox::critical(0, "Ошибка", "Не возможно сохранить QSO. Ошибка базы данных!", QMessageBox::Ok);
      return;
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ClearQso() {
  ui->callInput->clear();
  ui->gridsquareInput->clear();
  ui->cntyInput->clear();
  ui->qthInput->clear();
  ui->nameInput->clear();
  ui->commentInput->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::RemoveQSOs(QModelIndexList indexes) {
    int countRow = indexes.count();
    if (countRow == 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение действия", QString("Вы уверены что хотите удалить %1 QSO?").arg(countRow), QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        db.transaction();
        QSqlQuery query(db);
        query.prepare("DELETE FROM records WHERE id = ?");
        for (int i = countRow; i > 0; i--) {
            int id = RecordsModel->data(RecordsModel->index(indexes.at(i-1).row(), 0)).toInt();
            query.addBindValue(id);
            query.exec();
        }
        db.commit();

        RefreshRecords();
        //ScrollRecordsToBottom(); //BugFix
        ScrollRecordsToTop();
    } else {
        return;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onSettingsChanged() {
  QMessageBox::StandardButton confirm;
  confirm = QMessageBox::question(this, "Сохранение настроек", "Для применения настроек необходим перезапуск программы. Выполнить?", QMessageBox::Yes|QMessageBox::No);

  if (confirm == QMessageBox::Yes) {
      qApp->quit();
      QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::CallsignToUppercase(const QString &arg) {
    QString callsign = arg.toUpper();
    ui->callInput->setText(callsign);
    ui->callInput->setStyleSheet("font-weight: bold");

    CallTypeTimer->start();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::UpdateFormDateTime() {
    QDateTime DateTimeNow = QDateTime::currentDateTimeUtc().toUTC();
    QDate DateNow = QDate::currentDate();
    ui->dateInput->setDate(DateNow);
    ui->timeEdit->setTime(DateTimeNow.time());
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

void MainWindow::onStationCallsignChanged() {
  auto data = ui->stationCallsignCombo->itemData(ui->stationCallsignCombo->currentIndex()).value<QList<QVariant>>();
  userData.callsign_id = data.value(0).toInt();
  userData.qsosu_callsign_id = data.value(1).toInt();
  userData.callsign = ui->stationCallsignCombo->currentText();
  userData.gridsquare = data.value(3).toString();
  userData.cnty = data.value(4).toString();

  ui->qthlocEdit->setText(userData.gridsquare);
  ui->rdaEdit->setText(userData.cnty);

  SetRecordsFilter(userData.callsign_id);
  RefreshRecords();
  //ScrollRecordsToBottom(); //BugFix
  ScrollRecordsToTop();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onOperatorChanged() {
  auto data = ui->operatorCombo->itemData(ui->operatorCombo->currentIndex()).value<QList<QVariant>>();
  userData.qsosu_operator_id = data.value(1).toInt();
  userData.oper = ui->operatorCombo->currentText();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onUdpLogged() {
  // if (ui->stationCallsignCombo->currentIndex() == 0) {
  //   QMessageBox::critical(0, "Ошибка", "UDP: Не выбран позывной станции", QMessageBox::Ok);
  //   return;
  // }
  // if (ui->operatorCombo->currentIndex() == 0) {
  //   QMessageBox::critical(0, "Ошибка", "UDP: Не выбран позывной оператора", QMessageBox::Ok);
  //   return;
  // }

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
  newRecord.setValue("QSO_DATE", udpReceiver->time_off.date().toString("yyyyMMdd"));
  newRecord.setValue("TIME_OFF", udpReceiver->time_off.time().toString("hhmm") + "00");
  newRecord.setValue("TIME_ON", udpReceiver->time_on.time().toString("hhmm") + "00");
  newRecord.setValue("CALL", QString::fromUtf8(udpReceiver->dx_call));
  newRecord.setValue("BAND", Helpers::GetBandByFreqHz(udpReceiver->tx_frequency_hz));
  newRecord.setValue("FREQ", udpReceiver->tx_frequency_hz);
  newRecord.setValue("MODE", QString::fromUtf8(udpReceiver->mode));
  newRecord.setValue("NAME", QString::fromUtf8(udpReceiver->name));
  newRecord.setValue("QTH", "");
  newRecord.setValue("GRIDSQUARE", QString::fromUtf8(udpReceiver->dx_grid));
  newRecord.setValue("RST_SENT", QString::fromUtf8(udpReceiver->report_sent));
  newRecord.setValue("RST_RCVD", QString::fromUtf8(udpReceiver->report_received));
  newRecord.setValue("COMMENT", QString::fromUtf8(udpReceiver->comments));

  newRecord.setValue("sync_state", 0);

  if(RecordsModel->insertRecord(-1, newRecord)) {
      if (RecordsModel->submitAll()) {
        qDebug() << "New record submited to database";

        int LastID = RecordsModel->query().lastInsertId().toInt();
        QVariantList data;
        QString datetime = udpReceiver->time_on.date().toString("yyyy-MM-dd") + "T" + udpReceiver->time_on.time().toString("hh:mm:00");
        data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
        data << QString::fromUtf8(udpReceiver->dx_call) << Helpers::GetBandByFreqHz(udpReceiver->tx_frequency_hz) << QString::fromUtf8(udpReceiver->mode);
        data << udpReceiver->tx_frequency_hz << datetime << QString::fromUtf8(udpReceiver->name) << QString::fromUtf8(udpReceiver->report_sent) << QString::fromUtf8(udpReceiver->report_received) << "" << "" << QString::fromUtf8(udpReceiver->dx_grid);
        api->SendQso(data);

        RefreshRecords();
        //ScrollRecordsToBottom(); //BugFix
        ScrollRecordsToTop();
        ClearQso();
      }
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::FindCallDataQrzru() {
  QString callsign = ui->callInput->text().trimmed();
  if (callsign.isEmpty()) {
      ClearCallbookFields();
      return;
  }

  QStringList data = qrz->Get(callsign);
  ui->nameInput->setText((data.at(0).length() > 0) ? data.at(0) : "");
  ui->qthInput->setText((data.at(1).length() > 0) ? data.at(1) : "");
  ui->gridsquareInput->setText((data.at(2).length() > 0) ? data.at(2).toUpper() : "");
  ui->cntyInput->setText((data.at(3).length() > 0) ? data.at(3).toUpper() : "");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ClearCallbookFields() {
  ui->nameInput->clear();
  ui->qthInput->clear();
  ui->gridsquareInput->clear();
  ui->cntyInput->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::fillDefaultFreq() {
  //double freqMhz = Helpers::BandToDefaultFreqMHz(ui->bandCombo->currentText());
  double freqMhz = BandToDefaultFreq(ui->bandCombo->currentText());
  ui->freqInput->setText(QString::number(freqMhz, 'f', 6));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SyncQSOs(QModelIndexList indexes) {
    QStringList idstrings;
    for (int i = 0; i < indexes.size(); i++){
        idstrings << indexes[i].data().toString();
    }
    QString numberlist = idstrings.join(",");
    QSqlQuery query(db);
    query.exec(QString("SELECT id, qsosu_callsign_id, qsosu_operator_id, CALL, BAND, MODE, FREQ, QSO_DATE, TIME_OFF, NAME, RST_SENT, RST_RCVD, QTH, CNTY, GRIDSQUARE MY_CNTY, MY_GRIDSQUARE FROM records WHERE id IN (%1) ORDER BY id").arg(numberlist));
    while (query.next()) {
        int dbid = query.value(0).toInt();
        int qsosu_callsign_id = query.value(1).toInt();
        int qsosu_operator_id = query.value(2).toInt();
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
        data << dbid << qsosu_callsign_id << qsosu_operator_id << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare << my_cnty << my_gridsquare;
        api->SendQso(data);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::onQsoSynced(int dbid) {
    QSqlQuery query(db);
    if(query.exec(QString("UPDATE records SET sync_state = 1 WHERE id=%1").arg(dbid))) {
        RefreshRecords();
        ScrollRecordsToTop();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::readXmlfile()
{
    QDomDocument HamDefs;
    QFile xmlFile("HamDefs.xml");

    if(!xmlFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error open HamDefs.xml";
        return;
    }
    qDebug() << "Succesful open HamDefs.xml";

    if(!HamDefs.setContent(&xmlFile))
    {
        qDebug() << "Error read HamDefs.xml";
        return;
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
            ui->bandCombo->addItem(Band.attribute("band_name"));
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
            ui->modeCombo->addItem(Mode.attribute("mode_name"));
        }
    }
    ui->bandCombo->blockSignals(false);
    ui->modeCombo->blockSignals(false);
    ui->stationCallsignCombo->blockSignals(false);

    ui->bandCombo->setCurrentText(settings->lastBand);
    ui->modeCombo->setCurrentText(settings->lastMode);
    ui->qthlocEdit->setText(settings->lastLocator);
    ui->rdaEdit->setText(settings->lastRDA);
    ui->freqInput->setText(settings->lastFrequence);
    ui->rstrInput->setText(settings->lastRST_RCVD);
    ui->rstsInput->setText(settings->lastRST_SENT);

    ui->stationCallsignCombo->setCurrentIndex(settings->lastCallsign);
    ui->operatorCombo->setCurrentIndex(settings->lastOperator);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::LoadHamDefs()
{
    qDebug() << "Upload HamDefs.xml from QSO.SU";
    //api->getListBand(); //Загрузка диапазонов
    //api->getListSubmodeDropDown(); //Загрузка списков модуляции

    api->loadHamDefs(); //Попытка загрузки HamDefs.xml с сайта
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setModesList()
{
    ui->modeCombo->clear();
    if(api->modulations.count() > 0) ui->modeCombo->addItems(api->modulations);
    qInfo() << "Upload modes: " << api->modulations.count();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::setBandsList()
{
    ui->bandCombo->clear();
    if(api->bands.count() > 0) ui->bandCombo->addItems(api->bands);
    qInfo() << "Upload bands: " << api->bands.count();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::HamDefsUploaded()
{
    ui->bandCombo->clear();
    ui->modeCombo->clear();

    QDomDocument HamDefsDoc;
    HamDefsDoc.setContent(api->XMLdata);

    QFile file("HamDefs.xml");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    HamDefsDoc.save(out, 0);
    readXmlfile();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::HamDefsError()
{
    qDebug() << "Open HamDefs.xml flom local file...";
    readXmlfile();
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

void MainWindow::on_bandCombo_currentTextChanged(const QString &arg1)
{
    settings->lastBand = arg1;
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_modeCombo_currentTextChanged(const QString &arg1)
{
    QString report;
    settings->lastMode = arg1;
    SaveFormData();

    report = getRepotValueFromMode(arg1);

    if(report != "") {
        ui->rstrInput->setText(report);
        ui->rstsInput->setText(report);
    }
    //qDebug() << "Mode changed " << settings->lastMode;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SaveFormData()
{
    settings->lastLocator = ui->qthlocEdit->text();
    settings->lastRDA = ui->rdaEdit->text();
    settings->lastFrequence = ui->freqInput->text();
    settings->lastRST_RCVD = ui->rstrInput->text();
    settings->lastRST_SENT = ui->rstsInput->text();
    settings->saveForm();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::SaveCallsignState()
{
    settings->lastCallsign = ui->stationCallsignCombo->currentIndex();
    settings->lastOperator = ui->operatorCombo->currentIndex();
    settings->saveForm();
    //qDebug() << " Save lastCallsign " << ui->stationCallsignCombo->currentIndex();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_freqInput_editingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_rstrInput_editingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_rstsInput_editingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::darkTheime()
{
    // Создаём палитру для тёмной темы оформления
    QPalette darkPalette;

    // Настраиваем палитру для цветовых ролей элементов интерфейса
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

    // Устанавливаем данную палитру
    qApp->setPalette(darkPalette);
}
//------------------------------------------------------------------------------------------------------------------------------------------




//void MainWindow::on_actionLogger_CAT_triggered()
//{
    //logcat = new loggercat(this);
    //loggercat->show();
//}

