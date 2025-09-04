#ifndef EXPORTADIF_H
#define EXPORTADIF_H

#include <QDialog>
#include <QSqlDatabase>
#include <QTextStream>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QThread>

namespace Ui {
class ExportADIF;
}

class ExportADIF : public QDialog
{
    Q_OBJECT

public:
    explicit ExportADIF(QSqlDatabase dbconn, QWidget *parent = nullptr);
    ~ExportADIF();

    void ExportAll(QString callsign, QString oper);
    void Export(QString callsign, QString oper, QString data_from, QString data_to);
    void WriteFile(QSqlQuery query, QString suffix);
    QSqlDatabase db;

private slots:
    void on_BetweenRadioButton_toggled(bool checked);
    void on_AllRadioButton_toggled(bool checked);
    void on_pushButton_clicked();
    void on_ExportButton_clicked();

private:
    Ui::ExportADIF *ui;
    int total_inserted;
    void getCallsigns();

};

#endif // EXPORTADIF_H
