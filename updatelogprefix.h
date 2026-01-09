#ifndef UPDATELOGPREFIX_H
#define UPDATELOGPREFIX_H

#include <QDialog>
#include <QSqlRecord>
#include <QSqlQuery>

struct PrefixEntry {
    QString country;
    QString country_code;
    QString continent;
    QString cqzone;
    QString ituzone;
    QString dxcc;
    QString latitude;
    QString longitude;
    QStringList regexList;
};

namespace Ui {
class UpdateLogPrefix;
}

class UpdateLogPrefix : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateLogPrefix(QSqlDatabase db, QList<PrefixEntry> entries, QWidget *parent = nullptr);
    ~UpdateLogPrefix();
    PrefixEntry* findPrefixEntry(const QList<PrefixEntry>& entries, const QString& callsign);

private slots:
    void on_updateButton_clicked();
    void on_closeButton_clicked();

signals:
    void db_updated();

private:
    Ui::UpdateLogPrefix *ui;
    QSqlDatabase db;
    int dbid;
    QList<PrefixEntry> entries;
};

#endif // UPDATELOGPREFIX_H
