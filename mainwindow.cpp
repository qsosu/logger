#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "delegations.h"
#include "QRegularExpressionValidator"
#include "qlistview.h"
#include <QCompleter>

#define SET_ROBOT_FONT

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setWindowTitle("QSO.SU клиент");

  QFontDatabase::addApplicationFont("://resources/fonts/Roboto-Regular.ttf");

  ui->callInput->setStyleSheet("color: black; font-weight: bold");
  //BugFix Только латинские символы и цифры
  ui->callInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9_]*$"), this));

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

  ui->modeCombo->setEditable(true); // включаем встроенный QLineEdit
  ui->modeCombo->setInsertPolicy(QComboBox::NoInsert); // отключаем вставку новых элементов из QLineEdit
  ui->modeCombo->completer()->setCompletionMode(QCompleter::CompletionMode::PopupCompletion); // устанавливаем модель автодополнения (по умолчанию стоит InlineCompletition)

  LoadHamDefs(); //Загрузка XML-файла с диапазонами и модуляциями

  //Проверка использования и версий SSL
  qDebug() << "Support SSL: " << QSslSocket::supportsSsl() << " SSL Build Library: " << QSslSocket::sslLibraryBuildVersionString() << " SSL Library Version: " << QSslSocket::sslLibraryVersionString();

  settings = new Settings();
  settings->setAttribute(Qt::WA_QuitOnClose, false);
  connect(settings, SIGNAL(SettingsChanged()), this, SLOT(onSettingsChanged()));
  connect(ui->actionSettings, &QAction::triggered, this, [=]() {
      settings->show();
  });

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
      ScrollRecordsToBottom();
  });
  connect(ui->bandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(fillDefaultFreq()));
  connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customMenuRequested(QPoint)));

  connect(ui->actionAbout, &QAction::triggered, this, [=]() {
      QMessageBox::about(0,
                         tr("О программе"),
                         tr("Desktop API клиент сервиса QSO.SU\n\n"
                            "Версия ПО: 1.2.362, версия API: 1.0\n"
                            "Авторы: Alexey.K (R2SI)\n"
                            "Ильдар.М (R9JAU)\n"
                            "Артём.С (R4CAT)")
                         );
      return;
  });
  connect(ui->actionQsosuLink, &QAction::triggered, this, [=]() {
      QDesktopServices::openUrl(QUrl("https://qso.su/"));
  });
  connect(ui->actionQsosuFaqLink, &QAction::triggered, this, [=]() {
      QDesktopServices::openUrl(QUrl("https://qso.su/ru/faq"));
  });

  InitRecordsTable();
  getCallsigns();
  fillDefaultFreq();

  qInfo() << "Application started";
}

MainWindow::~MainWindow() {
  delete ui;
}

/* Database section */

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

bool MainWindow::CheckDatabase() {
    QFileInfo check_file(database_file);
    if (check_file.exists() && check_file.isFile()) return true;
    return false;
}

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

bool MainWindow::ConnectDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(database_file);
    if (!db.open()) return false;
    return true;
}

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

void MainWindow::getCallsigns() {
  ui->stationCallsignCombo->clear();
  ui->stationCallsignCombo->addItem("- Не выбран -", 0);
  ui->operatorCombo->clear();
  ui->operatorCombo->addItem("- Не выбран -", 0);

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
      ui->operatorCombo->addItem(name, QList<QVariant>() << id << qsosu_id);
  }
}

void MainWindow::InitRecordsTable() {
  RecordsModel = new QSqlTableModel(this);
  RecordsModel->setTable("records");

  //RecordsModel->setEditStrategy(QAbstractItemView::NoEditTriggers);

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
  ui->tableView->resizeColumnsToContents();

  ui->tableView->setItemDelegateForColumn(8, new FormatCallsign(ui->tableView));
  ui->tableView->setItemDelegateForColumn(9, new FormatDate(ui->tableView));
  ui->tableView->setItemDelegateForColumn(10, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(11, new FormatTime(ui->tableView));
  ui->tableView->setItemDelegateForColumn(13, new FormatFreq(ui->tableView));
  ui->tableView->setItemDelegateForColumn(22, new FormatSyncState(ui->tableView));

  ui->tableView->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView *horizontalHeader = ui->tableView->horizontalHeader();
  horizontalHeader->setSectionResizeMode(QHeaderView::Interactive);
  horizontalHeader->setMinimumSectionSize(90);
  horizontalHeader->setStretchLastSection(true);
  horizontalHeader->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));

  QHeaderView *verticalHeader = ui->tableView->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setDefaultSectionSize(20);

  ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tableView->setFont(QFont("Roboto", settings->fontSize, QFont::Normal, false));
}

void MainWindow::SetRecordsFilter(int log_id) {
    RecordsModel->setFilter(QString("callsign_id=%1").arg(log_id));
    RecordsModel->setSort(1, Qt::AscendingOrder); // Sort by ID
}

void MainWindow::RefreshRecords() {
    RecordsModel->select();
    while (RecordsModel->canFetchMore()) // FIX. Fetch more than 256 records!!!
        RecordsModel->fetchMore();
}

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

void MainWindow::SaveQso() {
  if (ui->stationCallsignCombo->currentIndex() == 0) {
    QMessageBox::critical(0, "Ошибка", "Не выбран позывной станции", QMessageBox::Ok);
    return;
  }
  if (ui->operatorCombo->currentIndex() == 0) {
    QMessageBox::critical(0, "Ошибка", "Не выбран позывной оператора", QMessageBox::Ok);
    return;
  }
  if (ui->callInput->text().length() == 0) {
      QMessageBox::critical(0, "Ошибка", "Не указан позывной корреспондента!", QMessageBox::Ok);
      ui->callInput->setFocus();
      return;
  }

  //qDebug() << "USER DATA:" << userData.callsign_id << userData.qsosu_callsign_id << userData.qsosu_operator_id << userData.callsign << userData.oper << userData.gridsquare << userData.cnty;

  QSqlRecord newRecord = RecordsModel->record();
  newRecord.remove(newRecord.indexOf("id"));
  newRecord.setValue("callsign_id", userData.callsign_id);
  newRecord.setValue("qsosu_callsign_id", userData.qsosu_callsign_id);
  newRecord.setValue("qsosu_operator_id", userData.qsosu_operator_id);
  newRecord.setValue("STATION_CALLSIGN", userData.callsign);
  newRecord.setValue("OPERATOR", userData.oper);
  newRecord.setValue("MY_GRIDSQUARE", ui->qthlocEdit->text().toUpper());
  newRecord.setValue("MY_CNTY", ui->rdaEdit->text().toUpper());
  QString call = ui->callInput->text();
  newRecord.setValue("CALL", call);

  //QDate qsoDate = QDate::fromString(ui->dateInput->text(), "yyyy-MM-dd");
  QDate qsoDate = ui->dateInput->date();
  newRecord.setValue("QSO_DATE", qsoDate.toString("yyyyMMdd"));

  QTime qsoTime = QTime::fromString(ui->timeInput->text(), "hh:mm:ss");
  QString datetime = ui->dateInput->text() + "T" + ui->timeInput->text();
  QString qsoTimeFormated = qsoTime.toString("hhmmss");
  newRecord.setValue("TIME_ON", qsoTimeFormated);
  newRecord.setValue("TIME_OFF", qsoTimeFormated);

  //QString band = ui->bandCombo->currentText();
  QString band = getBandValue(ui->bandCombo->currentIndex());
  newRecord.setValue("BAND", band);

  unsigned long long freqHz = static_cast<unsigned long long>(ui->freqInput->text().toDouble() * 1000000);
  newRecord.setValue("FREQ", freqHz);

  //QString mode = ui->modeCombo->currentText();
  QString mode = getModeValue(ui->modeCombo->currentIndex());
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

  if (RecordsModel->insertRecord(-1, newRecord)) {
      RecordsModel->submitAll();

      // Здесь надо отправлять QSO на сервис через API
      int LastID = RecordsModel->query().lastInsertId().toInt();
      QVariantList data;
      data << LastID << userData.qsosu_callsign_id << userData.qsosu_operator_id;
      data << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare;
      api->SendQso(data);

      RefreshRecords();
      ScrollRecordsToBottom();
      ClearQso();

      ui->callInput->setFocus();
  } else {
      QMessageBox::critical(0, "Ошибка", "Не возможно сохранить QSO. Ошибка базы данных!", QMessageBox::Ok);
      return;
  }
}

void MainWindow::ClearQso() {
  ui->callInput->clear();
  ui->gridsquareInput->clear();
  ui->cntyInput->clear();
  ui->qthInput->clear();
  ui->nameInput->clear();
  ui->commentInput->clear();
}

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
        ScrollRecordsToBottom();
    } else {
        return;
    }
}


void MainWindow::onSettingsChanged() {
  QMessageBox::StandardButton confirm;
  confirm = QMessageBox::question(this, "Сохранение настроек", "Для применения настроек необходим перезапуск программы. Выполнить?", QMessageBox::Yes|QMessageBox::No);

  if (confirm == QMessageBox::Yes) {
      qApp->quit();
      QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
  }
}

void MainWindow::CallsignToUppercase(const QString &arg) {
    QString callsign = arg.toUpper();
    ui->callInput->setText(callsign);
    ui->callInput->setStyleSheet("font-weight: bold");

    CallTypeTimer->start();
}

void MainWindow::UpdateFormDateTime() {
    QDateTime DateTimeNow = QDateTime::currentDateTimeUtc().toUTC();
    QDate DateNow = QDate::currentDate();
    ui->dateInput->setDate(DateNow);
    ui->timeInput->setText(DateTimeNow.toString("hh:mm:ss"));
}

void MainWindow::ScrollRecordsToBottom() {
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->selectRow(RecordsModel->rowCount() - 1);
    ui->tableView->scrollToBottom();
}

void MainWindow::onCallsignsUpdated() {
  getCallsigns();
}

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
  ScrollRecordsToBottom();
}

void MainWindow::onOperatorChanged() {
  auto data = ui->operatorCombo->itemData(ui->operatorCombo->currentIndex()).value<QList<QVariant>>();
  userData.qsosu_operator_id = data.value(1).toInt();
  userData.oper = ui->operatorCombo->currentText();
}

void MainWindow::onUdpLogged() {
  if (ui->stationCallsignCombo->currentIndex() == 0) {
    QMessageBox::critical(0, "Ошибка", "UDP: Не выбран позывной станции", QMessageBox::Ok);
    return;
  }
  if (ui->operatorCombo->currentIndex() == 0) {
    QMessageBox::critical(0, "Ошибка", "UDP: Не выбран позывной оператора", QMessageBox::Ok);
    return;
  }

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
        ScrollRecordsToBottom();
        ClearQso();
      }
  }

}

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

void MainWindow::ClearCallbookFields() {
  ui->nameInput->clear();
  ui->qthInput->clear();
  ui->gridsquareInput->clear();
  ui->cntyInput->clear();
}

void MainWindow::fillDefaultFreq() {
  //double freqMhz = Helpers::BandToDefaultFreqMHz(ui->bandCombo->currentText());
  double freqMhz = BandToDefaultFreq(ui->bandCombo->currentText());
  ui->freqInput->setText(QString::number(freqMhz, 'f', 6));
}

void MainWindow::SyncQSOs(QModelIndexList indexes) {
    QStringList idstrings;
    for (int i = 0; i < indexes.size(); i++){
        idstrings << indexes[i].data().toString();
    }
    QString numberlist = idstrings.join(",");
    QSqlQuery query(db);
    query.exec(QString("SELECT id, qsosu_callsign_id, qsosu_operator_id, CALL, BAND, MODE, FREQ, QSO_DATE, TIME_OFF, NAME, RST_SENT, RST_RCVD, QTH, CNTY, GRIDSQUARE FROM records WHERE id IN (%1) ORDER BY id").arg(numberlist));
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
        QVariantList data;
        data << dbid << qsosu_callsign_id << qsosu_operator_id << call << band << mode << freqHz << datetime << name << rsts << rstr << qth << cnty << gridsquare;
        api->SendQso(data);
    }
}

void MainWindow::onQsoSynced(int dbid) {
    QSqlQuery query(db);
    if(query.exec(QString("UPDATE records SET sync_state = 1 WHERE id=%1").arg(dbid))) {
        RefreshRecords();
    }
}

void MainWindow::on_modeCombo_currentIndexChanged(int index)
{
    ui->rstrInput->setText(modeList[index].mode_report);
    ui->rstsInput->setText(modeList[index].mode_report);
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
    qInfo() << "Succesful loaded HamDefs.xml";

    if(!HamDefs.setContent(&xmlFile))
    {
        qDebug() << "Error read HamDefs.xml";
        return;
    }

    QDomElement root = HamDefs.firstChildElement();

    QDomNodeList BandNames = HamDefs.elementsByTagName("band");
    qInfo() << "Loaded bands: " << BandNames.count();

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
    qInfo() << "Loaded modes: " << ModeNames.count();

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
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool MainWindow::LoadHamDefs()
{
    ui->bandCombo->clear();
    ui->modeCombo->clear();
    readXmlfile();
    ui->bandCombo->setCurrentIndex(7);
    ui->modeCombo->setCurrentIndex(1);
    return true;
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
        if(index == bandList[j].band_id)
            return bandList[j].band_value;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString MainWindow::getModeValue(int index)
{
    for(int j = 0; j < modeList.count(); j++)
    {
        if(index == modeList[j].mode_id)
            return modeList[j].mode_value;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------------
