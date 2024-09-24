#include "callsigns.h"
#include "ui_callsigns.h"


Callsigns::Callsigns(QSqlDatabase db, HttpApi *api, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Callsigns),
  db(db),
  api(api)
{
  ui->setupUi(this);
  setWindowTitle("Управление позывными");
  ui->saveButton->setEnabled(false);
  ui->cancelButton->setEnabled(false);

  connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddPressed()));
  connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(onRemovePressed()));
  connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSavePressed()));
  connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancelPressed()));
  connect(ui->updateFromQsosu, SIGNAL(clicked(bool)), this, SLOT(requestQsosu()));

  init();
  updateTable();
}

Callsigns::~Callsigns()
{
  delete ui;
}

void Callsigns::init() {

  CallsignsModel = new QSqlTableModel(this);
  CallsignsModel->setTable("callsigns");
  CallsignsModel->setEditStrategy(QSqlTableModel::OnFieldChange);

  CallsignsModel->setHeaderData(2, Qt::Horizontal, tr("Тип")); //BugFix Add
  CallsignsModel->setHeaderData(3, Qt::Horizontal, tr("Позывной"));
  CallsignsModel->setHeaderData(6, Qt::Horizontal, tr("QTH локатор"));
  CallsignsModel->setHeaderData(7, Qt::Horizontal, tr("RDA"));
  CallsignsModel->setHeaderData(8, Qt::Horizontal, tr("ITU зона"));
  CallsignsModel->setHeaderData(9, Qt::Horizontal, tr("CQ зона"));

  ui->callTable->setModel(CallsignsModel);
  ui->callTable->setColumnHidden(0, true);
  ui->callTable->setColumnHidden(1, true);
  //ui->callTable->setColumnHidden(2, true); //BugFix for visible column Type
  ui->callTable->setColumnHidden(4, true);
  ui->callTable->setColumnHidden(5, true);

  ui->callTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->callTable->setStyleSheet("selection-background-color: rgb(201, 217, 233); selection-color: rgb(0, 0, 0);");
  ui->callTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui->callTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  ui->callTable->verticalHeader()->setDefaultSectionSize(20);
}

void Callsigns::updateTable() {
  CallsignsModel->select();
}

void Callsigns::onAddPressed() {
  int rowCount = CallsignsModel->rowCount(QModelIndex());
  CallsignsModel->insertRow(rowCount);
  QModelIndex index = CallsignsModel->index(rowCount, 2);
  ui->callTable->setCurrentIndex(index);
  ui->callTable->edit(index);
//---------------------------------------------------------------------------------------
  //Добавим QComboBox в поле Type
  // itemcombobox = qobject_cast<QComboBox*>(ui->callTable->indexWidget(index));
  // if (!itemcombobox) {
  //     itemcombobox = new QComboBox();
  //     ui->callTable->setIndexWidget(index, itemcombobox);
  // }
  // lst.clear();
  // lst<< "Основной" << "Дополнительный" << "Специальный";
  // itemcombobox->addItems(lst);
  // connect(itemcombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Callsigns::CallsignsEdit);
//---------------------------------------------------------------------------------------
  ui->addButton->setEnabled(false);
  ui->saveButton->setEnabled(true);
  ui->cancelButton->setEnabled(true);
}

void Callsigns::onRemovePressed() {
  QModelIndexList indexList = ui->callTable->selectionModel()->selectedIndexes();
  if (!indexList.isEmpty()) {
      int rowIndex = indexList.first().row();
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "Подтверждение действия", QString("Вы уверены что хотите удалить позывной \"%1\"?").arg(CallsignsModel->index(rowIndex, 3).data().toString()), QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) {
          QSqlQuery query;
          if (query.exec(QString("DELETE FROM records WHERE callsign_id='%1'").arg(CallsignsModel->index(rowIndex, 1).data().toInt()))) {
              CallsignsModel->removeRow(rowIndex);
          } else {
              QMessageBox::critical(0, "Ошибка", "Не получилось удалить позывной. Ошибка базы данных!", QMessageBox::Ok);
              return;
          }
          updateTable();
          emit updated();
      } else {
          return;
      }
  } else {
      QMessageBox::critical(0, "Ошибка", "Не выбран позывной для удаления.", QMessageBox::Ok);
      return;
  }

}

void Callsigns::onSavePressed() {
  int newRowIndex = CallsignsModel->rowCount(QModelIndex()) - 1;

  QString name = CallsignsModel->index(newRowIndex, 3).data().toString();
  QString gridsquare = CallsignsModel->index(newRowIndex, 6).data().toString();

  if (name.length() < 3) {
    QMessageBox::critical(0, "Ошибка", "Позывной сигнал не может быть меньше 3 символов", QMessageBox::Ok);
    return;
  }
  if (gridsquare.length() < 4) {
    QMessageBox::critical(0, "Ошибка", "QTH локатор не может быть меньше 4 символов", QMessageBox::Ok);
    return;
  }

  if (CallsignsModel->submitAll()) {
    updateTable();
    ui->addButton->setEnabled(true);
    ui->saveButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);

    emit updated();
  } else {
    QMessageBox::critical(0, "Ошибка", "Ошибка базы данных!", QMessageBox::Ok);
    return;
  }

}

void Callsigns::onCancelPressed() {
  updateTable();
  ui->addButton->setEnabled(true);
  ui->saveButton->setEnabled(false);
  ui->cancelButton->setEnabled(false);
}

void Callsigns::requestQsosu() {
  api->getCallsign();
}

void Callsigns::onCallsignsUpdated() {
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

      if (QMessageBox::question(this,
                                "Обновление позывного",
                                QString("Обновить данные для позывного %1?").arg(name),
                                QMessageBox::Yes|QMessageBox::No
                                ) == QMessageBox::No) continue;

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
        if (QMessageBox::question(this,
                                  "Добавление позывного",
                                  QString("Позывной %1 отсутствует в системе. Добавить?").arg(name),
                                  QMessageBox::Yes|QMessageBox::No
                                  ) == QMessageBox::No) continue;

      QSqlQuery insert;
      insert.prepare("INSERT INTO callsigns (id, qsosu_id, type, name, validity_start, validity_stop, gridsquare, cnty, ituz, cqz) VALUES (NULL, :qsosu_id, :type, :name, :validity_start, :validity_stop, :gridsquare, :cnty, :ituz, :cqz)");
      insert.bindValue(":qsosu_id", qsosu_id);
      insert.bindValue(":type", type);
      insert.bindValue(":name", name);
      insert.bindValue(":validity_start", validity_start);
      insert.bindValue(":validity_stop", validity_stop);
      insert.bindValue(":gridsquare", location);
      insert.bindValue(":cnty", rda);
      insert.bindValue(":ituz", ituz);
      insert.bindValue(":cqz", cqz);
      if (!insert.exec()) qDebug() << "Error while inserting new callsing in DB";
    }

    query.clear();
    updateTable();
  }

  QMessageBox::information(0, "Обновление данных", "Позывные обновлены с сервиса QSO.SU", QMessageBox::Ok);
  emit updated();
}


void Callsigns::CallsignsEdit(int indx) {
    if (indx == 1) { //when user select testdata
        itemcombobox->setEditable(true);
    }
    else{
      itemcombobox->setEditable(false);
    }
}



//CREATE TABLE "callsigns" (
//	"id"	INTEGER NOT NULL,
//	"qsosu_id"	INTEGER NOT NULL DEFAULT 0,
//	"type"	INTEGER NOT NULL DEFAULT 10,
//	"name"	TEXT NOT NULL,
//	"validity_start"	INTEGER NOT NULL DEFAULT 0,
//	"validity_stop"	INTEGER NOT NULL DEFAULT 0,
//	"gridsquare"	TEXT NOT NULL,
//	"cnty"	TEXT,
//	"ituz"	INTEGER NOT NULL DEFAULT 0,
//	"cqz"	INTEGER NOT NULL DEFAULT 0,
//	PRIMARY KEY("id" AUTOICREMENT)
//)


