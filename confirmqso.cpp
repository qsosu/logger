/**********************************************************************************************************
Description :  Implementation of the ConfirmQSO class, which accepts and stores confirmed QSOs in the
            :  database from the QSO.SU server.
Version     :  1.0.2
Date        :  04.07.2025
Author      :  R9JAU
Comments    :
***********************************************************************************************************/

#include "confirmqso.h"
#include "ui_confirmqso.h"
#include <QSqlError>


ConfirmQSO::ConfirmQSO(QSqlDatabase db, Settings *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmQSO)
{
    this->db = db;
    this->settings = settings;
    ui->setupUi(this);

    setWindowTitle(tr("Подтверждение связей с QSO.SU"));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    api = new HttpApi(db, settings->accessToken);
    connect(api, SIGNAL(confirmQSOs(int)), this, SLOT(ConfirmQSOs(int)));
    getCallsigns();

    QDate cnfrm_date = QDate::fromString(getLastConfirmDate(), "yyyyMMdd");
    ui->dateEdit->setDate(cnfrm_date);
    ui->ConfirmProgressBar->setValue(0);
}
//------------------------------------------------------------------------------------------------------------------------------------------

ConfirmQSO::~ConfirmQSO()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::on_CloseCloseButton_clicked()
{
    ui->ConfirmProgressBar->setValue(0);
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::ConfirmQSOs(int count)
{
    ui->QSOCountLabel->setText(tr("Всего подтвержденных QSO: ") + QString::number(count));
    int cnfrm_count = 0;
    ui->ConfirmProgressBar->setMaximum(count);

    dbid = getMaxID();

    QList<QVariantList> batch;   // список строк для вставки
    batch.reserve(count);        // чтобы избежать лишних realocations

    for (QVariantMap &cnfrm : api->cnfrQSOs)
    {
        QString band            = cnfrm["band"].toString();
        QString band_type       = cnfrm["band_type"].toString();
        QString call            = cnfrm["call"].toString();
        QString cnty            = cnfrm["cnty"].toString();
        QString gridsquare      = cnfrm["gridsquare"].toString();
        QString hash            = cnfrm["hash"].toString();
        QString mode            = cnfrm["mode"].toString();
        QString my_cnty         = cnfrm["my_cnty"].toString();
        QString my_gridsquare   = cnfrm["my_gridsquare"].toString();
        QString st_oper         = cnfrm["operator"].toString();
        QString qso_date        = cnfrm["qso_date"].toString();
        QString rst_rcvd        = cnfrm["rst_rcvd"].toString();
        QString rst_sent        = cnfrm["rst_sent"].toString();
        QString station_callsign= cnfrm["station_callsign"].toString();
        QString time_off        = cnfrm["time_off"].toString();
        QString time_on         = cnfrm["time_on"].toString();

        QSqlQuery query(db);
        query.prepare("UPDATE RECORDS SET BAND=:band, CALL=:call, CNTY=:cnty, MODE=:mode, "
                      "MY_CNTY=:my_cnty, MY_GRIDSQUARE=:my_gridsquare, OPERATOR=:st_oper, "
                      "QSO_DATE=:qso_date, RST_RCVD=:rst_rcvd, RST_SENT=:rst_sent, "
                      "STATION_CALLSIGN=:station_callsign, TIME_OFF=:time_off, TIME_ON=:time_on, "
                      "sync_state = 1 WHERE HASH=:hash");

        query.bindValue(":band", band + band_type);
        query.bindValue(":call", call);
        query.bindValue(":cnty", cnty);
        query.bindValue(":gridsquare", gridsquare);
        query.bindValue(":mode", mode);
        query.bindValue(":my_cnty", my_cnty);
        query.bindValue(":my_gridsquare", my_gridsquare);
        query.bindValue(":st_oper", st_oper);
        query.bindValue(":qso_date", qso_date);
        query.bindValue(":rst_rcvd", rst_rcvd);
        query.bindValue(":rst_sent", rst_sent);
        query.bindValue(":station_callsign", station_callsign);
        query.bindValue(":time_off", time_off);
        query.bindValue(":time_on", time_on);
        query.bindValue(":hash", hash);

        if (!query.exec()) {
            qDebug() << "ERROR Confirmation QSO. " << query.lastError().text();
        } else {
            if (query.numRowsAffected() == 0 && ui->insertCheckBox->isChecked()) {
                QVariantList values;
                values << call << cnty << gridsquare << (band + band_type) << mode
                       << station_callsign << st_oper << my_cnty << my_gridsquare
                       << qso_date << time_on << time_off << rst_rcvd << rst_sent << hash;

                batch.append(values); // добавляем в список для последующей вставки
            }

            cnfrm_count++;
            dbid++;
            ui->ConfirmProgressBar->setValue(cnfrm_count);
            ui->CurrentQSOCountLabel->setText(tr("Загружено подтвержденных QSO: ") + QString::number(cnfrm_count));
            QApplication::processEvents();
        }
    }

    // если что-то накопилось — вставляем
    if (!batch.isEmpty()) {
        InsertQso(batch);
    }

    qDebug() << "Confirmed: " << cnfrm_count << " QSOs";
    emit db_updated();
    QMessageBox::information(this, tr("Подтверждение связей с QSO.SU"), tr("Подтверждено QSO: ") + QString::number(cnfrm_count));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::InsertQso(const QList<QVariantList> &batch)
{
    if (batch.isEmpty())
        return;

    QSqlQuery query(db);

    db.transaction();  // начинаем транзакцию

    query.prepare("INSERT INTO RECORDS "
                  "(CALL, CNTY, GRIDSQUARE, BAND, MODE, STATION_CALLSIGN, OPERATOR, "
                  "MY_CNTY, MY_GRIDSQUARE, QSO_DATE, TIME_ON, TIME_OFF, "
                  "RST_RCVD, RST_SENT, HASH, sync_state) "
                  "VALUES (:call, :cnty, :gridsquare, :band, :mode, :station_callsign, "
                  ":st_oper, :my_cnty, :my_gridsquare, :qso_date, :time_on, :time_off, "
                  ":rst_rcvd, :rst_sent, :hash, 1)");

    for (const QVariantList &row : batch) {
        query.bindValue(":call",            row[0]);
        query.bindValue(":cnty",            row[1]);
        query.bindValue(":gridsquare",      row[2]);
        query.bindValue(":band",            row[3]);
        query.bindValue(":mode",            row[4]);
        query.bindValue(":station_callsign",row[5]);
        query.bindValue(":st_oper",         row[6]);
        query.bindValue(":my_cnty",         row[7]);
        query.bindValue(":my_gridsquare",   row[8]);
        query.bindValue(":qso_date",        row[9]);
        query.bindValue(":time_on",         row[10]);
        query.bindValue(":time_off",        row[11]);
        query.bindValue(":rst_rcvd",        row[12]);
        query.bindValue(":rst_sent",        row[13]);
        query.bindValue(":hash",            row[14]);

        if (!query.exec()) {
            qDebug() << "Insert error: " << query.lastError().text();
        }
    }

    db.commit();  // коммитим транзакцию
}
//------------------------------------------------------------------------------------------------------------------------------------------

int ConfirmQSO::getMaxID()
{
    int value = 0;
    QSqlQuery query(db);
    query.exec("SELECT MAX(ID) FROM RECORDS");

    while(query.next()){
        value = query.value(0).toInt();
    }
    return value + 1;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int ConfirmQSO::getCallsignID(QString callsign)
{
    int value = 0;
    QSqlQuery query(db);
    query.exec(QString("SELECT qsosu_id FROM callsigns WHERE name = '%1'").arg(callsign));

    while(query.next()){
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int ConfirmQSO::getLocalCallsignID(QString callsign)
{
    int value = 0;
    QSqlQuery query(db);
    query.exec(QString("SELECT id FROM callsigns WHERE name = '%1'").arg(callsign));

    while(query.next()){
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::on_ConfirmButton_clicked()
{
    int id = getCallsignID(ui->CallSignComboBox->currentText());
    QDate qsoDate = ui->dateEdit->date();
    QString date = qsoDate.toString("yyyy-MM-dd");
    api->getConfirmedLogs(date, id);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::getCallsigns()
{
    QString callsign;
    ui->CallSignComboBox->clear();

    QSqlQuery query(db);
    query.exec("SELECT name FROM callsigns");
    while(query.next()) {
        callsign = query.value(0).toString();
        ui->CallSignComboBox->addItem(callsign);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString ConfirmQSO::getLastConfirmDate()
{
    QString value = "";
    QSqlQuery query(db);
    query.exec("SELECT DATE FROM LAST_CONFIRM");

    while(query.next()){
        value = query.value(0).toString();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::setLastConfirmDate(QString date)
{
    QSqlQuery query(db);
    query.prepare("UPDATE LAST_CONFIRM SET DATE = :date");
    query.bindValue(":date", date);

    if(!query.exec()){
        qDebug() << "ERROR UPDATE TABLE LAST_CONFIRM " << query.lastError().text();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::on_dateEdit_dateChanged(const QDate &date)
{
    QString last_date = date.toString("yyyyMMdd");
    setLastConfirmDate(last_date);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
