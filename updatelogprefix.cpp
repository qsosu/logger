#include "updatelogprefix.h"
#include "ui_updatelogprefix.h"
#include <QDebug>
#include <QSqlError>
#include "QRegularExpressionValidator"


UpdateLogPrefix::UpdateLogPrefix(QSqlDatabase db, QList<PrefixEntry> entries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateLogPrefix)
{
    this->db = db;
    this->entries = entries;
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->UpdateProgressBar->setValue(0);
}

UpdateLogPrefix::~UpdateLogPrefix()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UpdateLogPrefix::on_updateButton_clicked()
{
    int count = 0;
    int cnt = 0;

    QSqlQuery query(db);

    // Подсчет общего числа записей
    if (query.exec("SELECT COUNT(*) FROM RECORDS")) {
        if (query.next()) {
            count = query.value(0).toInt();
        }
    }
    qDebug() << "Records count: " << count;
    ui->UpdateProgressBar->setMaximum(count);

    // Получаем все ID и CALL
    if (!query.exec("SELECT ID, CALL FROM RECORDS")) {
        qDebug() << "Ошибка выборки записей:" << query.lastError().text();
        return;
    }

    // Определяем, какие столбцы обновляем
    QStringList setClauses;
    if (ui->country_cb->isChecked()) setClauses << "COUNTRY = :country";
    if (ui->country_code_cb->isChecked()) setClauses << "COUNTRY_CODE = :country_code";
    if (ui->continent_cb->isChecked()) setClauses << "CONT = :cont";
    if (ui->cqz_cb->isChecked()) setClauses << "CQZ = :cqz";
    if (ui->ituz_cb->isChecked()) setClauses << "ITUZ = :ituz";

    if (setClauses.isEmpty()) {
        qDebug() << "Ни один столбец не выбран для обновления.";
        return;
    }

    QString updateSql = "UPDATE RECORDS SET " + setClauses.join(", ") + " WHERE ID = :id";
    QSqlQuery update(db);
    if (!update.prepare(updateSql)) {
        qDebug() << "Ошибка подготовки запроса UPDATE:" << update.lastError().text();
        return;
    }

    db.transaction(); // ускоряем обновление

    while (query.next())
    {
        int id = query.value(0).toInt();
        QString Callsign = query.value(1).toString();

        PrefixEntry* result = findPrefixEntry(entries, Callsign);
        if (!result) {
            qDebug() << "Страна не найдена для позывного:" << Callsign;
            continue;
        }

        if (ui->country_cb->isChecked()) update.bindValue(":country", result->country);
        if (ui->country_code_cb->isChecked()) update.bindValue(":country_code", result->country_code);
        if (ui->continent_cb->isChecked()) update.bindValue(":cont", result->continent);
        if (ui->cqz_cb->isChecked()) update.bindValue(":cqz", result->cqzone);
        if (ui->ituz_cb->isChecked()) update.bindValue(":ituz", result->ituzone);

        update.bindValue(":id", id);

        if (!update.exec()) {
            qDebug() << "Ошибка обновления записи ID=" << id << ":" << update.lastError().text();
        }

        delete result;
        cnt++;

        // Обновление прогресс-бара каждые 50 записей
        if (cnt % 50 == 0) {
            ui->UpdateProgressBar->setValue(cnt);
            QApplication::processEvents();
        }
    }
    db.commit(); // завершаем транзакцию
    ui->UpdateProgressBar->setValue(count);

    qDebug() << "Обновлено записей:" << cnt;
    emit db_updated();
}

//------------------------------------------------------------------------------------------------------------------------------------------

PrefixEntry* UpdateLogPrefix::findPrefixEntry(const QList<PrefixEntry>& entries, const QString& callsign)
{
    QString csUpper = callsign.toUpper();

    for (const auto& entry : entries) {
        for (const auto& rawPattern : entry.regexList) {
            QString pattern = rawPattern;

            // Обработка экранирования: \Z => $ (конец строки)
            pattern.replace("\\Z", "$");

            // Явно указываем, что сравнение с начала строки
            if (!pattern.startsWith("^")) {
                pattern = "^" + pattern;
            }

            QRegularExpression re(pattern);
            if (!re.isValid()) {
                qDebug() << "Invalid regex:" << pattern;
                continue;
            }

            if (re.match(csUpper).hasMatch()) {
                return new PrefixEntry(entry);
            }
        }
    }
    return nullptr;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UpdateLogPrefix::on_closeButton_clicked()
{
    close();
}

