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
    QList<QString> rigRequesting();
    bool portOpened; //=false?
    void createSetFile();
    int CATinterval;
    QSerialPort serialPort;
    //QString rigFreqHz;
    //QString rigModeName;
    //QList<QString> info;


private slots:
    //QString rigRequesting(bool inf);
    void rigTmrSig();
    void saveCATset();
    void setupPort();


private:
    Ui::loggerCAT *ui;
    //void rigupd();
    //QSerialPort serialPort;
    QString path;
    QSettings *qs;
    void setRead();
    void openPath(QString path);
    //bool portOpened;
    //QVector<QString> modes={"no selection", "SSB (LSB)", "SSB (USB)", "CW", "FM", "AM", "FSK", "CW", "no selection", "FSK"};
    //QString rigFreq;
    //QString rigMode;
    //QString answ;
    //QString rigFreqHz;
    //QList<QString> info;

signals:
    void rigupd();
    void CATSettingsChanged();
    void portErr();

};

#endif // LOGGERCAT_H
