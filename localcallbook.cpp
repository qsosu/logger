#include "localcallbook.h"
#include <QDebug>

//------------------------------------------------------------------------------------------------------------------------------------------

LocalCallbook::LocalCallbook(QObject *parent)
    : QObject{parent}
{
    openDatabase();
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool LocalCallbook::openDatabase()
{
    const QString connectionName = "local_callbook";

    // если уже существует — удалить безопасно
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName("callbook.db");

    if (!db.open()) {
        qDebug() << "Не удалось открыть локальный Callbook:" << db.lastError().text();
        return false;
    }

    qInfo() << "Открыт локальный Callbook:" << db.databaseName();
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QStringList LocalCallbook::searchCall(QString callsign)
{
    QStringList data;

    QSqlDatabase db = QSqlDatabase::database("local_callbook");
    if (!db.isOpen()) {
        qDebug() << "База не открыта";
        return data << "" << "" << "" << "" << "";
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT "Call","Name","QTH","Grid","State"
        FROM Callbook
        WHERE UPPER("Call") = :call
    )");
    query.bindValue(":call", callsign.toUpper());

    if (!query.exec()) {
        qDebug() << "Ошибка запроса:" << query.lastError().text();
        return data << "" << "" << "" << "" << "";
    }

    if (query.next()) {
        data << query.value("Call").toString()
             << query.value("Name").toString()
             << query.value("QTH").toString()
             << query.value("Grid").toString()
             << query.value("State").toString();
    } else {
        data << "" << "" << "" << "" << "";
    }

    return data;
}
//------------------------------------------------------------------------------------------------------------------------------------------
