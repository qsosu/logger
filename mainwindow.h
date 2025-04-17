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
#include "udpserver.h"
#include "flrig.h"
#include "httpapi.h"
#include "helpers.h"
#include "qrzrucallbook.h"
#include "adif.h"
#include "about.h"
#include "apilogradio.h"
#include "delegations.h"
#include "qsoedit.h"
#include "cat_interface.h"

#define VERSION "2.0.3"

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

private:
  Ui::MainWindow *ui;
  Settings *settings;
  Qsoedit *qsoedit;
  Callsigns *callsigns;
  UdpServer *udpServer;
  Flrig *flrig;
  HttpApi *api;
  APILogRadio *logradio;
  cat_Interface *CAT;

  QString database_file;
  QSqlDatabase db;
  ColorSqlTableModel *RecordsModel;
  QTimer *EverySecondTimer;
  QrzruCallbook *qrz;
  QTimer *CallTypeTimer;
  QTimer *QsoSuPingTimer;
  Adif *adif;
  About *about;
  long freqCat;

  QLabel *qsosuLbl;
  QLabel *qsosuLabel;
  QLabel *udpserverLbl;
  QLabel *udpserverLabel;
  QLabel *flrigLbl;
  QLabel *flrigLabel;
  QLabel *catLabel;
  QLabel *catLbl;

  void InitDatabase(QString dbFile);
  bool CheckDatabase();
  bool ConnectDatabase();
  void CreateDatabase();
  void getCallsigns();
  void InitRecordsTable();
  void ScrollRecordsToBottom();
  void ScrollRecordsToTop();
  void FindCallData();
  void ClearCallbookFields();
  void RemoveQSOs(QModelIndexList indexes);
  void SetRecordsFilter(int log_id);
  void SyncQSOs(QModelIndexList indexes);
  void SaveFormData();
  void SaveCallsignState();
  void darkTheime();
  void RemoveDeferredQSOs();
  void insertDataToDeferredQSOs(int idx, QString hash);
  void EditQSO(QModelIndex index);
  void readXmlfile();
  double BandToDefaultFreq(QString band);
  QString getBandValue(int index);
  QString getModeValue(QString mode);
  QString getRepotValueFromMode(QString mode);
  int getSynchroStatus(int id);
  void PingQsoSu();

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void CallsignToUppercase(const QString &arg);
  void RefreshRecords();
  void SaveQso();
  void ClearQso();
  void UpdateFormDateTime();
  void fillDefaultFreq();
  void customMenuRequested(QPoint pos);
  void onQSOSUSynced(int dbid, QString hash);
  void onLogRadioSynced(int dbid);
  void LoadHamDefs();
  void setModesList();
  void setBandsList();
  void HamDefsUploaded();
  void HamDefsError();
  void setFreq(long freq);
  void setBand(int band);
  void setMode(int mode);
  void setUserData();
  void onQSOConfirmed();
  void onSettingsChanged();
  void onCallsignsUpdated();
  void onStationCallsignChanged();
  void onOperatorChanged();
  void onUdpLogged();
  void onUdpLoggedADIF();
  void on_bandCombo_currentTextChanged(const QString &arg1);
  void on_modeCombo_currentTextChanged(const QString &arg1);
  void on_freqInput_textChanged(const QString &arg1);
  void on_freqInput_editingFinished();
  void on_rstrInput_editingFinished();
  void on_rstsInput_editingFinished();
  void doubleClickedQSO(QModelIndex idx);
  void on_gridsquareInput_textEdited(const QString &arg1);
  void on_cntyInput_textEdited(const QString &arg1);
};
#endif // MAINWINDOW_H
