#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QHostAddress>
#include "apilogradio.h"

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
    QString logRadioAccessToken;
    bool udpServerEnable;
    uint16_t udpServerPort;
    QHostAddress flrigHost;
    uint16_t flrigPort;
    unsigned int flrigPeriod;
    bool enableQrzruCallbook;
    bool enableLogradio;
    QString QrzruLogin, QrzruPassword;
    QString LogradioLogin, LogradioPassword;
    unsigned int fontSize;
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
    void saveForm();

private:
    Ui::Settings *ui;
    QString path;
    QSettings *qs;
    APILogRadio *logradio;

    void openPath(QString path);
    void read();
    void createDefaultFile();
    void display();
    QString EncryptToken(QString data);
    QString DecryptToken(QString data);
    QString genSalt(QString data);

private slots:
    void save();
    void getLogRadioToken();
    void checkLogRadioToken();
    void checked(int code, QString message);
    void received(QString access_token, QString confirmation_key, QString confirmation_after, QString confirmation_before, QString valid_after, QString valid_before);

signals:
    void SettingsChanged();

};

#endif // SETTINGS_H
