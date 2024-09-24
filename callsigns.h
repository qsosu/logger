#ifndef CALLSIGNS_H
#define CALLSIGNS_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QComboBox>
#include "httpapi.h"

namespace Ui {
  class Callsigns;
}

class Callsigns : public QDialog
{
  Q_OBJECT

public:
  explicit Callsigns(QSqlDatabase db, HttpApi *api, QWidget *parent = nullptr);
  ~Callsigns();

private:
  Ui::Callsigns *ui;
  QSqlTableModel *CallsignsModel;
  QSqlDatabase db;
  HttpApi *api;
  QComboBox* itemcombobox;
  QStringList lst;

  void init();
  void updateTable();

public slots:
  void onCallsignsUpdated();

private slots:
  void onAddPressed();
  void onRemovePressed();
  void onSavePressed();
  void onCancelPressed();
  void requestQsosu();
  void CallsignsEdit(int indx);

  signals:
  void updated();

};

#endif // CALLSIGNS_H
