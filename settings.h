#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QHostAddress>

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
    bool udpServerEnable;
    uint16_t udpServerPort;
    QHostAddress flrigHost;
    uint16_t flrigPort;
    unsigned int flrigPeriod;
    bool enableQrzruCallbook;
    QString QrzruLogin, QrzruPassword;
    unsigned int fontSize;
    QString lastBand;
    QString lastMode;

    void saveForm();

private:
    Ui::Settings *ui;
    QString path;
    QSettings *qs;

    void openPath(QString path);
    void read();
    void createDefaultFile();
    void display();

private slots:
    void save();

signals:
    void SettingsChanged();

};

#endif // SETTINGS_H
