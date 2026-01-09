#ifndef LOCALCALLBOOK_H
#define LOCALCALLBOOK_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <qsqlerror.h>

//------------------------------------------------------------------------------------------------------------------------------------------

class LocalCallbook : public QObject
{
    Q_OBJECT
public:
    explicit LocalCallbook(QObject *parent = nullptr);
    QStringList searchCall(QString call);

signals:

private:
    QSqlDatabase db;
    bool openDatabase();
};
//------------------------------------------------------------------------------------------------------------------------------------------
#endif // LOCALCALLBOOK_H
