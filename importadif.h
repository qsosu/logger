#ifndef IMPORTADIF_H
#define IMPORTADIF_H

#include <QDialog>
#include <QSqlDatabase>
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlQuery>
#include "updatelogprefix.h"

struct QSO {
    QString stationCallsign;
    QString oper;
    QString call;
    QString name;
    QString my_gridsquare;
    QString my_cnty;
    QString qsoDate;
    QString qsoDateOff;
    QString timeOn;
    QString timeOff;
    QString mode;
    QString rstSent;
    QString rstRcvd;
    QString band;
    QString freq;
    QString country;
    QString country_code;
    QString continent;
    QString contestID;
    QString state;
    QString city;
    QString gridsquare;
    QString comment;
    QString cqz;
    QString ituz;
};

struct QsoRecord {
    QMap<QString, QString> fields;
};

namespace Ui {
class ImportADIF;
}

class ImportADIF : public QDialog
{
    Q_OBJECT

public:
    explicit ImportADIF(QSqlDatabase dbconn, QVector<CountryEntry> entries, QWidget *parent = nullptr);
    ~ImportADIF();
    QSqlDatabase db;
    int dbid;
    QString fileName;

    void ReadFile();
    QList<QsoRecord> parse(const QString &filePath);
    void InsertQso(const QSO &qso);
    int getLocalCallsignID(QString callsign);
    int getCallsignID(QString callsign);

private slots:
    void on_importButton_clicked();
    void on_closeButton_clicked();
    void on_pushButton_clicked();

private:
    Ui::ImportADIF *ui;
    QVector<CountryEntry> entries;
    CountryEntry findCountryByCall(const QString &call, const QVector<CountryEntry> &cty);
    void getCallsigns();

signals:
    void db_updated();

};

#endif // IMPORTADIF_H
