#ifndef UPLOADINGLOGS_H
#define UPLOADINGLOGS_H

#include <QDialog>
#include "settings.h"
#include "httpapi.h"
#include "updatelogprefix.h"



namespace Ui {
class UploadingLogs;
}

class UploadingLogs : public QDialog
{
    Q_OBJECT

public:
    explicit UploadingLogs(QSqlDatabase db, Settings *settings, QList<PrefixEntry> entries, QWidget *parent = nullptr);
    ~UploadingLogs();

    int totalCount;
    int qso_count;
    int getMaxID();
    int getCallsignID(QString callsign);
    int getLocalCallsignID(QString callsign);
    void getCallsigns();
    void getCashCallsignID();

signals:
    void db_updated();

private slots:
    void UploadQSOs(int count);
    void on_CloseCloseButton_clicked();
    void on_UploadButton_clicked();

protected:
    void changeEvent(QEvent *event) override;
    bool event(QEvent *event) override;

private:
    Ui::UploadingLogs *ui;
    QSqlDatabase db;
    Settings *settings;
    QList<PrefixEntry> entries;
    HttpApi *api;
    int dbid;
    QMap<QString,int> callsignToID;
    QMap<QString,int> callsignToLocalID;
    QString normalizeFreq(const QString &freqStr, const QString &band);
    PrefixEntry* findPrefixEntry(const QList<PrefixEntry>& entries, const QString& callsign);
};

#endif // UPLOADINGLOGS_H
