#ifndef UPDATELOGPREFIX_H
#define UPDATELOGPREFIX_H

#include "ham_definitions.h"

#include <QDialog>
#include <QSqlRecord>
#include <QSqlQuery>


namespace Ui {
class UpdateLogPrefix;
}

class UpdateLogPrefix : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateLogPrefix(QSqlDatabase db, QVector<CountryEntry> entries, QWidget *parent = nullptr);
    ~UpdateLogPrefix();
    CountryEntry findCountryByCall(const QString &call, const QVector<CountryEntry> &cty);

private slots:
    void on_updateButton_clicked();
    void on_closeButton_clicked();

protected:
    void changeEvent(QEvent *event);

signals:
    void db_updated();

private:
    Ui::UpdateLogPrefix *ui;
    QSqlDatabase db;
    int dbid;
    QVector<CountryEntry> entries;
};

#endif // UPDATELOGPREFIX_H
