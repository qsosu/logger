/**********************************************************************************************************
Description :  UpdateLogPrefix dialog class for batch updating QSO records with DXCC/prefix info.
Version     :  1.2.0
Date        :  15.08.2025
Author      :  R9JAU
Comments    :
    - Uses a local cache of PrefixEntry objects containing regex patterns for prefix matching.
    - Efficiently updates the database using a single transaction for all records.
***********************************************************************************************************/


#include "updatelogprefix.h"
#include "ui_updatelogprefix.h"
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include "QRegularExpressionValidator"


UpdateLogPrefix::UpdateLogPrefix(QSqlDatabase db, QVector<CountryEntry> entries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateLogPrefix)
{
    this->db = db;
    this->entries = entries;
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->UpdateProgressBar->setValue(0);
}
//------------------------------------------------------------------------------------------------------------------------------------------

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


        CountryEntry result = findCountryByCall(Callsign, entries);
        if(!result.country.isEmpty())
        {
            if (ui->country_cb->isChecked()) update.bindValue(":country", result.country);
            if (ui->country_code_cb->isChecked()) update.bindValue(":country_code", countryToIso.value(result.country, ""));
            if (ui->continent_cb->isChecked()) update.bindValue(":cont", result.continent);
            if (ui->cqz_cb->isChecked()) update.bindValue(":cqz", result.cqZone);
            if (ui->ituz_cb->isChecked()) update.bindValue(":ituz", result.ituZone);
        }
        update.bindValue(":id", id);

        if (!update.exec()) {
            qDebug() << "Ошибка обновления записи ID=" << id << ":" << update.lastError().text();
        }
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
    QMessageBox::information(this, tr("Обновление территорий"), tr("Обновлено QSO: ") + QString::number(cnt));
}

//------------------------------------------------------------------------------------------------------------------------------------------

CountryEntry UpdateLogPrefix::findCountryByCall(const QString &call, const QVector<CountryEntry> &cty)
{
    QString upperCall = call.toUpper();
    CountryEntry best;
    int bestLen = -1;

    for (const auto &c : cty) {
        for (const QString &p : c.prefixes) {
            QString up = p.toUpper();
            if (upperCall.startsWith(up)) {
                if (up.length() > bestLen) {
                    best = c;
                    bestLen = up.length();
                }
            }
        }
    }
    return best;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UpdateLogPrefix::on_closeButton_clicked()
{
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UpdateLogPrefix::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
