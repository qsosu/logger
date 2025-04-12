#ifndef CALLSIGNS_H
#define CALLSIGNS_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QComboBox>
#include "httpapi.h"
#include "addcallsign.h"

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
  Addcallsign *add_cs;

  void init();
  void updateTable();

public slots:
  void onCallsignsUpdated();

private slots:
  void onAddPressed();
  void onRemovePressed();
  void onSavePressed();
  void requestQsosu();
  void addCallsigng();
  void on_checkCallsignBtn_clicked();
  void callStatus(int status);

signals:
  void updated();

};

#endif // CALLSIGNS_H
