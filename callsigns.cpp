#include "callsigns.h"
#include "ui_callsigns.h"
#include <QSqlError>


Callsigns::Callsigns(QSqlDatabase db, HttpApi *api, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Callsigns),
  db(db),
  api(api)
{
  ui->setupUi(this);
  setWindowTitle(tr("Управление позывными"));
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

  connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddPressed()));
  connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(onRemovePressed()));
  connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSavePressed()));
  connect(ui->updateFromQsosu, SIGNAL(clicked(bool)), this, SLOT(requestQsosu()));
  connect(api, SIGNAL(callsignStatus(int)), this, SLOT(callStatus(int)));

  init();
  updateTable();
}
//--------------------------------------------------------------------------------------------------------------------

Callsigns::~Callsigns()
{
  delete ui;
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::init() {

  CallsignsModel = new QSqlTableModel(this);
  CallsignsModel->setTable("callsigns");
  CallsignsModel->setEditStrategy(QSqlTableModel::OnFieldChange);

  CallsignsModel->setHeaderData(3, Qt::Horizontal, tr("Позывной"));
  CallsignsModel->setHeaderData(6, Qt::Horizontal, tr("QTH локатор"));
  CallsignsModel->setHeaderData(7, Qt::Horizontal, tr("RDA"));
  CallsignsModel->setHeaderData(8, Qt::Horizontal, tr("ITU зона"));
  CallsignsModel->setHeaderData(9, Qt::Horizontal, tr("CQ зона"));
  CallsignsModel->setHeaderData(10, Qt::Horizontal, tr("Статус"));

  ui->callTable->setModel(CallsignsModel);
  ui->callTable->setColumnHidden(0, true);
  ui->callTable->setColumnHidden(1, true);
  ui->callTable->setColumnHidden(2, true);
  ui->callTable->setColumnHidden(4, true);
  ui->callTable->setColumnHidden(5, true);

  ui->callTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->callTable->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");
  ui->callTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->callTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  ui->callTable->verticalHeader()->setDefaultSectionSize(20);
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::updateTable()
{
  CallsignsModel->select();
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::onAddPressed()
{

    add_cs = new Addcallsign(this);
    connect(add_cs, SIGNAL(addCallsign()), this, SLOT(addCallsigng()));
    add_cs->exec();
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::addCallsigng()
{
    int rowCount = CallsignsModel->rowCount(QModelIndex());
    CallsignsModel->insertRow(rowCount);
    QModelIndex index = CallsignsModel->index(rowCount, 1);
    ui->callTable->setCurrentIndex(index);
    //ui->callTable->edit(index);

    CallsignsModel->setData(CallsignsModel->index(rowCount, 2), add_cs->add_CallsignType);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 3), add_cs->add_Callsign);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 4), add_cs->add_validity_start);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 5), add_cs->add_validity_stop);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 6), add_cs->add_location);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 7), add_cs->add_rda);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 8), add_cs->add_ituz);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 9), add_cs->add_cqz);
    CallsignsModel->setData(CallsignsModel->index(rowCount, 10), add_cs->status);

    QVariantList data;
    data << add_cs->add_Callsign << add_cs->add_CallsignType << add_cs->add_location << add_cs->add_rda << add_cs->add_ituz << add_cs->add_cqz << add_cs->add_validity_start << add_cs->add_validity_stop;
    api->addCallsign(data);
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::onRemovePressed()
{
  QModelIndexList indexList = ui->callTable->selectionModel()->selectedIndexes();
  if (!indexList.isEmpty()) {
      int rowIndex = indexList.first().row();
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Подтверждение действия"), QString(tr("Вы уверены что хотите удалить позывной \"%1\"?")).arg(CallsignsModel->index(rowIndex, 3).data().toString()), QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) {
          QSqlQuery query;
          if (query.exec(QString("DELETE FROM records WHERE callsign_id='%1'").arg(CallsignsModel->index(rowIndex, 1).data().toInt()))) {
              CallsignsModel->removeRow(rowIndex);
          } else {
              QMessageBox::critical(0, tr("Ошибка"), tr("Не получилось удалить позывной. Ошибка базы данных!"), QMessageBox::Ok);
              return;
          }
          updateTable();
          emit updated();
      } else {
          return;
      }
  } else {
      QMessageBox::critical(0, tr("Ошибка"), tr("Не выбран позывной для удаления."), QMessageBox::Ok);
      return;
  }

}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::onSavePressed()
{
  int newRowIndex = CallsignsModel->rowCount(QModelIndex()) - 1;

  QString name = CallsignsModel->index(newRowIndex, 3).data().toString();
  QString gridsquare = CallsignsModel->index(newRowIndex, 6).data().toString();

  if (name.length() < 3) {
    QMessageBox::critical(0, tr("Ошибка"), tr("Позывной сигнал не может быть меньше 3 символов"), QMessageBox::Ok);
    return;
  }
  if (gridsquare.length() < 4) {
    QMessageBox::critical(0, tr("Ошибка"), tr("QTH локатор не может быть меньше 4 символов"), QMessageBox::Ok);
    return;
  }

  if (CallsignsModel->submitAll()) {
    updateTable();
    emit updated();
  } else {
    QMessageBox::critical(0, tr("Ошибка"), tr("Ошибка базы данных!"), QMessageBox::Ok);
    qDebug() << "Ошибка при добавлении позывного:" << CallsignsModel->lastError().text();
    return;
  }

}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::requestQsosu()
{
  api->getCallsign();
}
//--------------------------------------------------------------------------------------------------------------------

void Callsigns::onCallsignsUpdated()
{
  for(QVariantMap &call : api->callsigns) {
    QString name = call["name"].toString();
    int qsosu_id = call["id"].toInt();
    int type = call["type"].toInt();
    int validity_start = call["validity_start"].toInt();
    int validity_stop = call["validity_stop"].toInt();
    QString location = call["location"].toString();
    QString rda = call["rda"].toString();
    int ituz = call["ituz"].toInt();
    int cqz = call["cqz"].toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM callsigns WHERE name=:name");
    query.bindValue(":name", name);
    query.exec();
    query.first();

    if (query.at() + 1 == 1) {
      int dbid = query.value(0).toInt();

      QSqlQuery update;
      update.prepare("UPDATE callsigns SET qsosu_id=:qsosu_id, type=:type, validity_start=:validity_start, validity_stop=:validity_stop, gridsquare=:gridsquare, gridsquare=:gridsquare, cnty=:cnty, ituz=:ituz, cqz=:cqz WHERE id=:id");
      update.bindValue(":qsosu_id", qsosu_id);
      update.bindValue(":type", type);
      update.bindValue(":validity_start", validity_start);
      update.bindValue(":validity_stop", validity_stop);
      update.bindValue(":gridsquare", location);
      update.bindValue(":cnty", rda);
      update.bindValue(":ituz", ituz);
      update.bindValue(":cqz", cqz);

      update.bindValue(":id", dbid);
      update.exec();
      if (!update.exec()) qDebug() << "Error while updateing callsing in DB";
    } else {
      QSqlQuery insert;
      insert.prepare("INSERT INTO callsigns (id, qsosu_id, type, name, validity_start, validity_stop, gridsquare, cnty, ituz, cqz, status) VALUES (NULL, :qsosu_id, :type, :name, :validity_start, :validity_stop, :gridsquare, :cnty, :ituz, :cqz, :status)");
      insert.bindValue(":qsosu_id", qsosu_id);
      insert.bindValue(":type", type);
      insert.bindValue(":name", name);
      insert.bindValue(":validity_start", validity_start);
      insert.bindValue(":validity_stop", validity_stop);
      insert.bindValue(":gridsquare", location);
      insert.bindValue(":cnty", rda);
      insert.bindValue(":ituz", ituz);
      insert.bindValue(":cqz", cqz);
      insert.bindValue(":status", 0);
      if (!insert.exec()) qDebug() << "Error while inserting new callsing in DB";
    }
    query.clear();
    updateTable();
  }
  QMessageBox::information(0, tr("Обновление данных"), tr("Позывные обновлены с сервиса QSO.SU"), QMessageBox::Ok);
  emit updated();
}

//---------------------------------------------------------------------------------------------------------------------

void Callsigns::on_checkCallsignBtn_clicked()
{
    QModelIndexList indexList = ui->callTable->selectionModel()->selectedIndexes();
    if (!indexList.isEmpty()) {
        int rowIndex = indexList.first().row();
        CallSign = CallsignsModel->index(rowIndex, 3).data().toString();
        api->checkStatusCallsign(CallSign);
    } else {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не выбран позывной для проверки."), QMessageBox::Ok);
        return;
    }
}
//---------------------------------------------------------------------------------------------------------------------

void Callsigns::callStatus(int status)
{
    if(status == 0) {
        QMessageBox::information(0, tr("Проверка статуса позывного."), tr("Позывной на проверке!"), QMessageBox::Ok);
        updateStatus("На проверке");
        updateTable();
        return;
    }
    if(status == 1) {
        QMessageBox::information(0, tr("Проверка статуса позывного."), tr("Позывной добавлен!"), QMessageBox::Ok);
        updateStatus("Добавлен");
        updateTable();
        return;
    }
    if(status == 2) {
        QMessageBox::information(0, tr("Проверка статуса позывного."), tr("Позывной отклонен!"), QMessageBox::Ok);
        updateStatus("Отклонен");
        updateTable();
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------

void Callsigns::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Callsigns::updateStatus(QString status)
{
    QSqlQuery update;
    update.prepare("UPDATE callsigns SET status=:status WHERE name=:name");
    update.bindValue(":name", CallSign);
    update.bindValue(":status", status);
    update.exec();
    if (!update.exec()) qDebug() << "Error while updateing callsing in DB";
}
//---------------------------------------------------------------------------------------------------------------------


