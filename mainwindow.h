#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QFileInfo>
#include <QKeyEvent>
#include <QProcess>
#include <QTimer>
#include <QDesktopServices>
#include <QFontDatabase>

#include "settings.h"
#include "callsigns.h"
#include "udpreceiver.h"
#include "flrig.h"
#include "httpapi.h"
#include "helpers.h"
#include "qrzrucallbook.h"
#include "adif.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  Settings *settings;
  Callsigns *callsigns;
  UdpReceiver *udpReceiver;
  Flrig *flrig;
  HttpApi *api;
  QString database_file;
  QSqlDatabase db;
  QSqlTableModel *RecordsModel;
  QTimer *EverySecondTimer;
  QrzruCallbook *qrz;
  QTimer *CallTypeTimer;
  Adif *adif;

  typedef struct baseData {
    int callsign_id;
    int qsosu_callsign_id;
    int qsosu_operator_id;
    QString callsign;
    QString oper;
    QString gridsquare;
    QString cnty;
  } baseData_t;
  baseData_t userData;

  void InitDatabase(QString dbFile);
  bool CheckDatabase();
  bool ConnectDatabase();
  void CreateDatabase();
  void getCallsigns();
  void InitRecordsTable();
  void ScrollRecordsToBottom();
  void FindCallDataQrzru();
  void ClearCallbookFields();
  void RemoveQSOs(QModelIndexList indexes);
  void SetRecordsFilter(int log_id);
  void SyncQSOs(QModelIndexList indexes);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
  void onSettingsChanged();
  void CallsignToUppercase(const QString &arg);
  void RefreshRecords();
  void SaveQso();
  void ClearQso();
  void UpdateFormDateTime();
  void onCallsignsUpdated();
  void onStationCallsignChanged();
  void onOperatorChanged();
  void onUdpLogged();
  void fillDefaultFreq();
  void customMenuRequested(QPoint pos);
  void onQsoSynced(int dbid);

};
#endif // MAINWINDOW_H
