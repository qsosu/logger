#ifndef CONFIRMQSO_H
#define CONFIRMQSO_H

#include <QDialog>
#include "settings.h"
#include "httpapi.h"


namespace Ui {
class ConfirmQSO;
}

class ConfirmQSO : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmQSO(QSqlDatabase db, Settings *settings, QWidget *parent = nullptr);
    ~ConfirmQSO();

    int getCallsignID(QString callsign);
    int getLocalCallsignID(QString callsign);
    void InsertQso(QStringList qso_data);
    int getMaxID();
    void getCallsigns();
    QString getLastConfirmDate();
    void setLastConfirmDate(QString date);

signals:
    void db_updated();

private slots:
    void on_CloseCloseButton_clicked();
    void on_ConfirmButton_clicked();
    void ConfirmQSOs(int count);
    void on_dateEdit_dateChanged(const QDate &date);

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::ConfirmQSO *ui;
    QSqlDatabase db;
    Settings *settings;
    HttpApi *api;
    int dbid;
};

#endif // CONFIRMQSO_H
