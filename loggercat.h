#ifndef LOGGERCAT_H
#define LOGGERCAT_H

#include <QDialog>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QSerialPort>

namespace Ui {
class loggerCAT;
}

class loggerCAT : public QDialog
{
    Q_OBJECT

public:
    explicit loggerCAT(QWidget *parent = nullptr);
    ~loggerCAT();
    void rigRequesting();
    void createSetFile();
    int CATinterval;
    QSerialPort *serialPort;
    unsigned long long rigFreq;
    QString rigMode;



private slots:
    void rigTmrSig();
    void saveCATset();
    void setupPort();
    void reading();


private:
    Ui::loggerCAT *ui;
    QString path;
    QSettings *qs;
    void setRead();
    void openPath(QString path);
    //QVector<QString> modes={"no selection", "SSB (LSB)", "SSB (USB)", "CW", "FM", "AM", "FSK", "CW", "no selection", "FSK"};
    QByteArray answ;
    QByteArray rigAnswer;

signals:
    void rigupd();
    void CATSettingsChanged();
    void portErr();
    void updated();

};

#endif // LOGGERCAT_H
