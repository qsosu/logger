#ifndef ADIF_H
#define ADIF_H

#include <QObject>
#include <QSqlDatabase>
#include <QTextStream>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QThread>

class Adif : public QObject
{
    Q_OBJECT
public:
    Adif(QSqlDatabase dbconn);
    void Export(int callsign_id);
    void ExportPartial(QList<int> indexes);
    void WriteFile(QSqlQuery query, QString suffix);

    QSqlDatabase db;

private:
    int total_inserted;

private slots:

signals:

};

#endif // ADIF_H

