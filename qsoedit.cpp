#include "qsoedit.h"
#include "ui_qsoedit.h"
#include <QDebug>
#include <QSqlError>
#include <QPixmap>
#include <QList>


Qsoedit::Qsoedit(QSqlDatabase db, Settings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Qsoedit)
{
    this->db = db;
    this->settings = settings;
    ui->setupUi(this);
    setWindowTitle("Редактирование QSO");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    load_flag = false;

    //settings = new Settings();
    qrz = new QrzruCallbook(settings->QrzruLogin, settings->QrzruPassword);
    connect(qrz, &QrzruCallbook::error404, this, [=]() {
        QMessageBox::information(this, "Ответ QRZ.RU", tr("Позывной не найден!"));
    });
    connect(qrz, &QrzruCallbook::error, this, [=]() {
        QMessageBox::information(this, "Ответ QRZ.RU", tr("QRZ API - ошибка запроса!"));
    });
    connect(qrz, SIGNAL(loaded(QPixmap)), this, SLOT(loadImage(QPixmap)));

    api = new HttpApi(db, settings->accessToken);
    api->getListBand(); //Загрузка диапазонов
    api->getListSubmodeDropDown(); //Загрузка списков модуляции
    connect(api, SIGNAL(userDataUpdated()), this, SLOT(setUserData()));
    connect(api, SIGNAL(QSODataUpdated(QString)), this, SLOT(updateQSOData(QString)));
    connect(api, SIGNAL(errorQSODataUpdated(QString)), this, SLOT(errorUpdateQSOData(QString)));
    //connect(api, SIGNAL(confirmQSOs()), this, SLOT(onQSOConfirmed()));

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true); // Выполнить только один раз
    connect(resizeTimer, &QTimer::timeout, this, &Qsoedit::onResizeFinished);

    ui->QSOSUUserIcon->setVisible(false);
    ui->QSOSUUserLabel->setVisible(false);
    ui->SRRUserIcon->setVisible(false);
    ui->SRRUserLabel->setVisible(false);
    ui->countrylineEdit->setText("");
    ui->cqzlineEdit->setText("");
    ui->ituzlineEdit->setText("");
    ui->ituzlineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-9]|[1-8][0-9]|90)$"), this));
    ui->cqzlineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-9]|[1-3][0-9]|40)$"), this));
    ui->rstr_lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-5][1-9]|[1-5][1-9][1-9])$"), this));
    ui->rsts_lineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-5][1-9]|[1-5][1-9][1-9])$"), this));
}
//------------------------------------------------------------------------------------------------------------------------------------------

Qsoedit::~Qsoedit()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::ShowQSOParams(QVariantList data)
{
    getCallsigns();
    dbid = data.at(0).toInt();
    int callsign_id = data.at(1).toInt();
    int operator_id = data.at(2).toInt();
    ui->StationComboBox->setCurrentText(getCallsignName(callsign_id));
    ui->OperatorComboBox->setCurrentText(getCallsignName(operator_id));
    ui->CalsignlineEdit->setText(data.at(3).toString());
    QDate qsoDate = QDate::fromString(data.at(4).toString(), "yyyyMMdd");
    ui->dateEdit->setDate(qsoDate);
    QTime start_time = QTime::fromString(data.at(5).toString(), "hhmmss");
    ui->qso_timeStartEdit->setTime(start_time);
    QTime end_time = QTime::fromString(data.at(6).toString(), "hhmmss");
    ui->QSO_timeEndEdit->setTime(end_time);
    ui->name_lineEdit->setText(data.at(10).toString());
    ui->qth_lineEdit->setText(data.at(11).toString());
    ui->rstr_lineEdit->setText(data.at(12).toString());
    ui->rsts_lineEdit->setText(data.at(13).toString());
    ui->qthloc_lineEdit->setText(data.at(14).toString());
    ui->rda_lineEdit->setText(data.at(15).toString());
    ui->ituzlineEdit->setText(data.at(16).toString());
    ui->cqzlineEdit->setText(data.at(17).toString());
    ui->comment_lineEdit->setText(data.at(18).toString());
    ui->countrylineEdit->setText(data.at(19).toString());
    ui->countryCodelineEdit->setText(data.at(20).toString());
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::on_QRZUpdateButton_clicked()
{
    if(settings->useCallbook)
    {
        api->getCallbook(ui->CalsignlineEdit->text());
        if(userData.length() > 10) {
            ui->name_lineEdit->setText((userData.at(0).length() > 0) ? userData.at(0) : "");
            ui->qth_lineEdit->setText((userData.at(1).length() > 0) ? userData.at(1) : "");
            ui->qthloc_lineEdit->setText((userData.at(2).length() > 0) ? userData.at(2).toUpper() : "");
            ui->rda_lineEdit->setText((userData.at(3).length() > 0) ? userData.at(3).toUpper() : "");
            ui->countrylineEdit->setText((userData.at(8).length() > 0) ? userData.at(8).toUpper() : "");
            ui->countryCodelineEdit->setText((userData.at(9).length() > 0) ? userData.at(9).toUpper() : "");
            ui->ituzlineEdit->setText((userData.at(10).length() > 0) ? userData.at(10).toUpper() : "");
            ui->cqzlineEdit->setText((userData.at(11).length() > 0) ? userData.at(11).toUpper() : "");
        }
    } else {
        QStringList data = qrz->Get(ui->CalsignlineEdit->text());
        ui->name_lineEdit->setText((data.at(0).length() > 0) ? data.at(0) : "");
        ui->qth_lineEdit->setText((data.at(1).length() > 0) ? data.at(1) : "");
        ui->qthloc_lineEdit->setText((data.at(2).length() > 0) ? data.at(2).toUpper() : "");
        ui->rda_lineEdit->setText((data.at(3).length() > 0) ? data.at(3).toUpper() : "");
        image = ((data.at(4).length() > 0) ? data.at(4) : "");
        if(image != "") {
            qrz->LoadPhoto(image);
            load_flag = true;
        } else noneImage();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::setUserData()
{
  userData.clear();
  userData.append(api->callsignInfo);

  if(userData.at(4).length() > 0) {
      ui->QSOSUUserIcon->setVisible(true);
      ui->QSOSUUserLabel->setVisible(true);
      if(userData.at(4) == "1") {
         ui->QSOSUUserIcon->setPixmap(QPixmap(":resources/images/loguser.png"));
         ui->QSOSUUserLabel->setText("Пользователь QSO.SU");
         ui->QSOSUUserLabel->setStyleSheet("QLabel { font-weight: bold; color: rgb(25, 135, 84) }");
     } else {
         ui->QSOSUUserIcon->setPixmap(QPixmap(":resources/images/no_loguser.png"));
         ui->QSOSUUserLabel->setText("Не пользователь QSO.SU");
         ui->QSOSUUserLabel->setStyleSheet("QLabel { font-weight: bold; color: rgb(220, 53, 69) }");
     }
  } else {
      ui->QSOSUUserIcon->setVisible(false);
      ui->QSOSUUserLabel->setVisible(false);
  }

  if(userData.at(5).length() > 0) {
      ui->SRRUserIcon->setVisible(true);
      ui->SRRUserLabel->setVisible(true);
      if(userData.at(5) == "1") {
         ui->SRRUserIcon->setPixmap(QPixmap(":resources/images/srr_user.png"));
         ui->SRRUserLabel->setText("Член СРР");
         ui->SRRUserLabel->setStyleSheet("QLabel { font-weight: bold; color: rgb(25, 135, 84) }");
     } else {
         ui->SRRUserIcon->setVisible(false);
         ui->SRRUserLabel->setVisible(false);
     }
  } else {
      ui->SRRUserIcon->setVisible(false);
      ui->SRRUserLabel->setVisible(false);
  }

  image = ((userData.at(12).length() > 0) ? userData.at(12) : "");
  if(image != "") {
      qrz->LoadPhoto(image);
      load_flag = true;
  } else noneImage();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::loadImage(QPixmap pix)
{
    pixmap_item = new QGraphicsPixmapItem();
    scene = new QGraphicsScene;
    ui->graphicsView->setScene(scene);
    scene->addItem(pixmap_item);
    pixmap_item->setVisible(true);
    pixmap_item->setPixmap(pix);
    scene->setSceneRect(0, 0, pix.width(), pix.height());
    ui->graphicsView->fitInView(pixmap_item, Qt::KeepAspectRatio);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::noneImage()
{
    scene = new QGraphicsScene;
    ui->graphicsView->setScene(scene);
    textItem = scene->addText("Нет фотографии!");
    QFont font("Arial", 24);
    textItem->setFont(font);
    textItem->setDefaultTextColor(Qt::lightGray);
    textItem->setPos(100, 100);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::on_saveButton_clicked()
{
    int callsign_id = getCallsignID(ui->StationComboBox->currentText());
    int operator_id = getCallsignID(ui->OperatorComboBox->currentText());
    int local_callsign_id = getLocalCallsignID(ui->StationComboBox->currentText());

    qDebug() << "Local Callsign ID= " << local_callsign_id;
    QSqlQuery query(db);
    query.prepare("UPDATE records SET CALL = :call, CALLSIGN_ID = :local_callsign_id, QSOSU_CALLSIGN_ID = :callsign_id, QSOSU_OPERATOR_ID = :operator_id, NAME = :name, COUNTRY = :country, CONT = :cont, QSO_DATE = :qso_date, "
                  "TIME_ON = :time_on, TIME_OFF = :time_off, QTH = :qth, GRIDSQUARE = :grid, CNTY = :rda, RST_SENT = :rsts, RST_RCVD = :rstr, ITUZ = :ituz, CQZ = :cqz WHERE id=:id");
    query.bindValue(":id", dbid);
    query.bindValue(":call", ui->CalsignlineEdit->text());
    query.bindValue(":local_callsign_id", local_callsign_id);
    query.bindValue(":callsign_id", callsign_id);
    query.bindValue(":operator_id", operator_id);
    query.bindValue(":name",  ui->name_lineEdit->text());
    query.bindValue(":country",  ui->countrylineEdit->text());
    query.bindValue(":cont",  ui->countryCodelineEdit->text());
    QDate qsoDate = ui->dateEdit->date();
    query.bindValue(":qso_date",  qsoDate.toString("yyyyMMdd"));
    QTime qsoTimeOn = ui->qso_timeStartEdit->time();
    QString qsoTimeOnFormated = qsoTimeOn.toString("hhmmss");
    QTime qsoTimeEnd = ui->QSO_timeEndEdit->time();
    QString qsoTimeEndFormated = qsoTimeEnd.toString("hhmmss");
    query.bindValue(":time_on",  qsoTimeOnFormated);
    query.bindValue(":time_off", qsoTimeEndFormated);
    query.bindValue(":qth",  ui->qth_lineEdit->text());
    query.bindValue(":grid",  ui->qthloc_lineEdit->text());
    query.bindValue(":rda",  ui->rda_lineEdit->text());
    query.bindValue(":rsts",  ui->rsts_lineEdit->text());
    query.bindValue(":rstr",  ui->rstr_lineEdit->text());
    query.bindValue(":ituz",  ui->ituzlineEdit->text());
    query.bindValue(":cqz",  ui->cqzlineEdit->text());

    if(!query.exec()){
        qDebug() << "ERROR UPDATE TABLE record " << query.lastError().text();
     } else {
        qDebug() << "Record " << dbid << " updated.";
        emit db_updated();
     }
    QString hash = getHashByID(dbid);
    QString callsign = ui->CalsignlineEdit->text();

    QString api_date = qsoDate.toString("yyyy-MM-dd");
    QString api_qsoTimeOn = api_date + "T" + qsoTimeOn.toString("hh:mm:ss");
    QString api_qsoTimeEnd = api_date + "T" + qsoTimeEnd.toString("hh:mm:ss");

    QVariantList data;
    data << hash << operator_id << callsign_id << callsign << ui->name_lineEdit->text() << ui->qth_lineEdit->text() << ui->rda_lineEdit->text() << ui->qthloc_lineEdit->text();
    data << ui->rsts_lineEdit->text() << ui->rstr_lineEdit->text() << ui->cqzlineEdit->text() << ui->ituzlineEdit->text() << api_qsoTimeOn << api_qsoTimeEnd << ui->countrylineEdit->text() << ui->countryCodelineEdit->text();
    api->updateByHashLog(data);
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::on_cancelButton_clicked()
{
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::closeEvent(QCloseEvent *event)
{
    if(load_flag) {
        scene->removeItem(pixmap_item);
        scene->deleteLater();
        scene = nullptr;
        delete pixmap_item;
        load_flag = false;
    }
    ui->QSOSUUserIcon->setVisible(false);
    ui->QSOSUUserLabel->setVisible(false);
    ui->SRRUserIcon->setVisible(false);
    ui->SRRUserLabel->setVisible(false);
    event->accept();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    resizeTimer->start(300); // Установить задержку в 300 мс
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::onResizeFinished()
{
    if(image != "") {
        qrz->LoadPhoto(image);
        load_flag = true;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::setVisible(bool set)
{
    QDialog::setVisible( set );
    if( set ) {
        if(settings->useCallbook) api->getCallbook(ui->CalsignlineEdit->text());
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::getCallsigns()
{
  ui->StationComboBox->clear();
  ui->OperatorComboBox->clear();

  QSqlQuery query(db);
  query.exec("SELECT qsosu_id, type, name FROM callsigns");
  while(query.next()) {
      qsosu_id = query.value(0).toInt();
      type = query.value(1).toInt();
      QString name = query.value(2).toString();
      ui->StationComboBox->addItem(name);
      if(type == 0) ui->OperatorComboBox->addItem(name);
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString Qsoedit::getCallsignName(int id)
{
    QString value;
    QSqlQuery query(db);
    query.exec(QString("SELECT name FROM callsigns WHERE qsosu_id = %1").arg(id));

    while(query.next()){
        value = query.value(0).toString();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int Qsoedit::getLocalCallsignID(QString callsign)
{
    int value = 0;
    QSqlQuery query(db);
    query.exec(QString("SELECT id FROM callsigns WHERE name = '%1'").arg(callsign));

    while(query.next()){
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int Qsoedit::getCallsignID(QString callsign)
{
    int value = 0;
    QSqlQuery query(db);
    query.exec(QString("SELECT qsosu_id FROM callsigns WHERE name = '%1'").arg(callsign));

    while(query.next()){
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString Qsoedit::getHashByID(int db_id)
{
    QString hash;
    QSqlQuery query(db);
    query.exec(QString("SELECT HASH FROM records WHERE id = '%1'").arg(db_id));

    while(query.next()){
        hash = query.value(0).toString();
    }
    return hash;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::updateQSOData(QString hash)
{
    QSqlQuery query(db);
    query.prepare("UPDATE records SET HASH = :hash WHERE id=:id");
    query.bindValue(":id", dbid);
    query.bindValue(":hash", hash);

    if(!query.exec()){
        qDebug() << "ERROR update HASH for record: " << dbid << query.lastError().text();
    } else {
        qDebug() << "HASH for record " << dbid << " updated.";
        QMessageBox::information(0, "Обновление QSO", "QSO успешно обновлено.", QMessageBox::Ok);
        emit db_updated();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Qsoedit::errorUpdateQSOData(QString error)
{
    QMessageBox::critical(0, "Ошибка", error, QMessageBox::Ok);
}
//------------------------------------------------------------------------------------------------------------------------------------------

//void Qsoedit::onQSOConfirmed()
//{
//    qDebug() << "Test confirm slot!!";

//}
////------------------------------------------------------------------------------------------------------------------------------------------





