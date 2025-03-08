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
#include "about.h"
#include "loggercat.h" //or <loggercat.h>?


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  struct bandData {
      int band_id;
      QString band_name;
      QString band_value;
      QString band_freq;
  };
  QList<bandData> bandList;

  struct modeData {
      int mode_id;
      QString mode_name;
      QString mode_value;
      QString mode_report;
  };
  QList<modeData> modeList;

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
  About *about;
  loggerCAT *loggercat;

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
  void ScrollRecordsToTop();
  void FindCallDataQrzru();
  void ClearCallbookFields();
  void RemoveQSOs(QModelIndexList indexes);
  void SetRecordsFilter(int log_id);
  void SyncQSOs(QModelIndexList indexes);
  void SaveFormData();
  void SaveCallsignState();
  void darkTheime();

  //bool LoadHamDefs();
  void readXmlfile();
  double BandToDefaultFreq(QString band);
  QString getBandValue(int index);
  QString getModeValue(QString mode);
  QString getRepotValueFromMode(QString mode);

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
  void LoadHamDefs();
  void setModesList();
  void setBandsList();
  void HamDefsUploaded();
  void HamDefsError();
  void on_bandCombo_currentTextChanged(const QString &arg1);
  void on_modeCombo_currentTextChanged(const QString &arg1);
  void on_freqInput_editingFinished();
  void on_rstrInput_editingFinished();
  void on_rstsInput_editingFinished();
  //void on_actionLogger_CAT_triggered();
};
#endif // MAINWINDOW_H
