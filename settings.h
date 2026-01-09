#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QHostAddress>
#include "apilogradio.h"
#include "httpapi.h"
#include "cat_interface.h"
#include "geolocation.h"


//Макросы для управления битами данных
#define biton(x,y) (x|=1<<y)
#define bitoff(x,y) (x&=~(1<<y))
#define testbit(x,y) (x&(1<<y))


namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

    QString accessToken;
    QString premium;
    QString logRadioAccessToken;
    bool udpServerEnable;
    uint16_t udpServerPort;
    bool udpClientEnable;
    uint16_t udpClientPort;
    bool proxyEnable;
    QString proxyUserName;
    QString proxyUserPassword;
    uint16_t proxyType;
    uint16_t proxyHTTPSPort;
    QHostAddress proxyHost;
    QHostAddress flrigHost;
    uint16_t flrigPort;
    unsigned int flrigPeriod;
    bool enableQrzruCallbook;
    bool enableLogradio;
    QString QrzruLogin, QrzruPassword;
    QString LogradioLogin, LogradioPassword;
    unsigned int fontSize;
    QString language;
    bool catEnable;
    QString trxType;
    int catInterval;
    QString serialPort;
    QString serialPortBaud;
    QString serialPortDataBits;
    QString serialPortStopBit;
    QString serialPortParity;
    QString serialPortFlowControl;
    QString lastBand;
    QString lastMode;
    QString lastFrequence;
    int lastCallsign;
    int lastOperator;
    QString lastRDA;
    QString lastLocator;
    QString lastRST_SENT;
    QString lastRST_RCVD;
    bool darkTheime;
    bool useCallbook;
    bool useLocalCallbook;
    bool showMap;
    double Latitude;
    double Longitude;
    unsigned int table_row_state;
    void saveForm();
    void read();
    void display();

private:
    Ui::Settings *ui;
    QString path;
    QSettings *qs;
    APILogRadio *logradio;
    cat_Interface *CAT;
    QTranslator qtLanguageTranslator;
    QStringList qmFiles;        // Список файлов переводов
    QStringList languageNames;  // Список названий языков

    void openPath(QString path);
    void createDefaultFile();
    uint saveTableState();
    QString EncryptToken(QString data);
    QString DecryptToken(QString data);
    QString genSalt(QString data);
    void loadTranslations();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void save();
    void getLogRadioToken();
    void checkLogRadioToken();
    void checked(int code, QString message);
    void received(QString access_token, QString confirmation_key, QString confirmation_after, QString confirmation_before, QString valid_after, QString valid_before);
    void getUserInfo(QStringList data);
    void on_CallbookCheckBox_toggled(bool checked);
    void on_qrzruEnable_toggled(bool checked);
    void on_languageComboBox_currentIndexChanged(int index);

signals:
    void SettingsChanged();

};

#endif // SETTINGS_H
