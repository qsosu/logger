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
#include "qsopanel.h"
#include "callsigns.h"
#include "udpserver.h"
#include "flrig.h"
#include "httpapi.h"
#include "helpers.h"
#include "qrzrucallbook.h"
#include "exportadif.h"
#include "importadif.h"
#include "about.h"
#include "apilogradio.h"
#include "delegations.h"
#include "qsoedit.h"
#include "cat_interface.h"
#include "confirmqso.h"
#include "localcallbook.h"
#include "uploadinglogs.h"
#include "geolocation.h"
#include "updatelogprefix.h"
#include "reports/reportcountry.h"
#include "reports/reportcontinent.h"
#include "reports/reportsunchart.h"
#include "reports/reportbands.h"
#include "reports/reportmodes.h"
#include "reports/reportyear.h"
#include "telnetclient.h"
#include "spotviewer.h"
#include "ham_definitions.h"
#include "chatcontroller.h"
#include "globe.h"


#define VERSION "3.0.1"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

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

  QList<bandData> bandList;
  QList<modeData> modeList;

private:
  Ui::MainWindow *ui;
  QSOPanel *qsoPanel;
  Settings *settings;
  QTranslator qtLanguageTranslator;
  Qsoedit *qsoedit;
  Callsigns *callsigns;
  UdpServer *udpServer;
  Flrig *flrig;
  HttpApi *api;
  APILogRadio *logradio;
  cat_Interface *CAT;
  ConfirmQSO *confirmQSO;
  UploadingLogs *uploadLogs;
  Geolocation *osm;
  Globe *globe;
  GlobeContainer *gc;
  ChatController *chats;
  bool COMPortAvailable;
  QString lastCallsign;
  //QMap<QString, PrefixEntry> prefixMap;

  QString database_file;
  QSqlDatabase db;
  ColorSqlTableModel *RecordsModel;
  ColorSqlTableModel *PrevRecordsModel;
  QrzruCallbook *qrz;
  QTimer *QsoSuPingTimer;
  QTimer *QrzRuPingTimer;
  QTimer *MagStormTimer;
  ExportADIF *exp_adif;
  ImportADIF *imp_adif;
  LocalCallbook *localCallbook;
  QString lastDxCall;
  About *about;
  long freqCat;
  UpdateLogPrefix *update_prefix;
  ReportCountry *reportCountry;
  ReportContinent *reportContinent;
  ReportSunChart *reportSunChart;
  ReportBands *reportBands;
  ReportModes *reportModes;
  ReportYear *reportYear;
  TelnetClient *tclient;
  SpotViewer *spotViewer;
  bool spotServerConnected = false;
  bool hasNewMessages = false;
  bool hasNewNews = false;
  QVector<CountryEntry> ctyData;
  bool serviceWasAvailable = false;

  QLabel *qsosuLbl;
  QLabel *qsosuLabel;
  QLabel *udpserverLbl;
  QLabel *udpserverLabel;
  QLabel *flrigLbl;
  QLabel *flrigLabel;
  QLabel *catLabel;
  QLabel *catLbl;
  QLabel *countQSO;
  QLabel *magStormLabel;
  QPushButton *bellBtn;
  QPushButton *notificationBtn;

  void InitDatabase(QString dbFile);
  void ReinitSettings();
  bool CheckDatabase();
  bool ConnectDatabase();
  void CreateDatabase();
  void getCallsigns();
  void InitRecordsTable();
  void InitPreviosQSOModel();
  void ScrollRecordsToBottom();
  void ScrollRecordsToTop();
  void FindCallData(const QString &callsign);
  bool findInLocal(const QString &callsign);
  bool findInQrz(const QString &callsign);
  void setUserData();
  void RemoveQSOs(QModelIndexList indexes);
  void SetRecordsFilter(int log_id);
  void SyncQSOs(QModelIndexList indexes);
  void SaveFormData();
  void SaveCallsignState();
  void ShowQSOLocation(QString callsign, QString locator);
  void lightTheme();
  void darkTheme();
  void RemoveDeferredQSOs();
  void insertDataToDeferredQSOs(int idx, QString hash);
  void EditQSO(QModelIndex index);
  void SendSpotQSO(QModelIndex index);
  bool LoadHamDefsSync();
  bool readXmlfile();
  double BandToDefaultFreq(QString band);
  QString getBandValue(int index);
  QString getModeValue(QString mode);
  QString getRepotValueFromMode(QString mode);
  int getSynchroStatus(int id);
  void PingQsoSu();
  void PingQrzRu();
  void ShowQSOInMap();
  void showPreviosQSO(QString call);
  void setLanguage();
  void loadCallList();
  void setTableRow();
  QModelIndex findIndexById(QSqlTableModel *model, int id, int targetColumn = 0);
  void MagStormUpdate();
  void saveHeaderState(QTableView *tableView);
  void restoreHeaderState(QTableView *tableView);
  QVector<CountryEntry> loadCtyDatabase(const QString &fileName);
  CountryEntry findCountryByCall(const QString &call, const QVector<CountryEntry> &cty);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void RefreshRecords();
  void SaveQso();
  void fillDefaultFreq();
  void customMenuRequested(QPoint pos);
  void prevMenuRequested(QPoint pos);
  void onQSOSUSynced(int dbid, QString hash);
  void onLogRadioSynced(int dbid);
  void setFreq(long freq);
  void setBand(int band);
  void setMode(int mode);
  void onQsoSuResult(bool found);
  void onSettingsChanged();
  void onCallsignsUpdated();
  void onStationCallsignChanged();
  void onOperatorChanged();
  void onModeChanged(QString mode);
  void onUdpLogged();
  void onUdpLoggedADIF();
  void UpdateMeasurement(QJsonObject data);
  void showBellIcon();
  void showNotificationIcon();
  void openNewsWindow();
  void setSpotInfo(QString call, QString band, double freq, QString mode);
  void doubleClickedQSO(QModelIndex idx);
  void doubleClickedPrevQSO(QModelIndex idx);
  void on_actionConfirmQSOs_triggered();
  void on_actionShowMap_triggered();
  void on_tableView_clicked(const QModelIndex &index);
  void on_actionPreviosQSO_triggered(bool checked);
  void on_actionUploadQSOs_triggered();
  void on_actionUpdatePrefix_triggered();
  void on_actionCheckCountry_triggered();
  void on_actionReportCountry_triggered();
  void on_actionReportContinent_triggered();
  void on_actionReportSun_triggered();
  void on_actionReportBands_triggered();
  void on_actionReportModes_triggered();
  void on_actionShowSpots_triggered();
  void on_actionImportADIF_triggered();
  void on_actionExportADIF_triggered();
  void on_actionChats_triggered();
  void on_actionShowLogLocation_triggered();
  void on_actionReportYear_triggered();
  void on_actionGlobe_triggered();
};
#endif // MAINWINDOW_H
