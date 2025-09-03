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

    for(QVariantMap &cnfrm : api->cnfrQSOs)
    {
        QString band = cnfrm["band"].toString();
        QString band_type = cnfrm["band_type"].toString();
        QString call = cnfrm["call"].toString();
        QString cnty = cnfrm["cnty"].toString();
        QString gridsquare = cnfrm["gridsquare"].toString();
        QString hash = cnfrm["hash"].toString();
        QString mode = cnfrm["mode"].toString();
        QString my_cnty = cnfrm["my_cnty"].toString();
        QString my_gridsquare = cnfrm["my_gridsquare"].toString();
        QString st_oper = cnfrm["operator"].toString();
        QString qso_date = cnfrm["qso_date"].toString();
        QString rst_rcvd = cnfrm["rst_rcvd"].toString();
        QString rst_sent = cnfrm["rst_sent"].toString();
        QString station_callsign = cnfrm["station_callsign"].toString();
        //QString submode = cnfrm["submode"].toString();
        QString time_off = cnfrm["time_off"].toString();
        QString time_on = cnfrm["time_on"].toString();

        QSqlQuery query(db);
        query.prepare("UPDATE RECORDS SET BAND=:band, CALL=:call, CNTY=:cnty, MODE=:mode, MY_CNTY=:my_cnty, MY_GRIDSQUARE=:my_gridsquare,"
                      "OPERATOR=:st_oper, QSO_DATE=:qso_date, RST_RCVD=:rst_rcvd, RST_SENT=:rst_sent, STATION_CALLSIGN=:station_callsign,"
                      "TIME_OFF=:time_off, TIME_ON=:time_on, sync_state = 1 WHERE HASH=:hash");
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
        }
        else {
            QStringList qso_data;
            qso_data << call << cnty << gridsquare << band + band_type << mode << station_callsign << st_oper << my_cnty << my_gridsquare << qso_date << time_on << time_off << rst_rcvd << rst_sent << hash;
            if(query.numRowsAffected() == 0 && ui->insertCheckBox->checkState()) InsertQso(qso_data);
            cnfrm_count++;
            dbid++;
            ui->ConfirmProgressBar->setValue(cnfrm_count);
            ui->CurrentQSOCountLabel->setText(tr("Загружено подтвержденных QSO: ") + QString::number(cnfrm_count));
            QApplication::processEvents();
            query.clear();
        }
    }
    qDebug() << "Confirmed: " << cnfrm_count << " QSOs";
    emit db_updated();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ConfirmQSO::InsertQso(QStringList qso_data)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO RECORDS (id, callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF,"
                  "BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT, sync_state, HASH, ITUZ, CQZ, SYNC_QSO) "
                  "VALUES (:id, :callsign_id, :qsosu_callsign_id, :qsosu_operator_id, :station_callsign, :operator, :my_gridsquare, :my_cnty, :call, :qso_date, :time_on, :time_off,"
                  ":band, :freq, :mode, :rst_sent, :rst_rcvd, :name, :qth, :gridsquare, :cnty, :comment, :sync_state, :hash, :ituz, :cqz, :sync_qso)");
    query.bindValue(":id", dbid);
    query.bindValue(":callsign_id", getLocalCallsignID(qso_data.at(5)));
    query.bindValue(":qsosu_callsign_id", getCallsignID(qso_data.at(5)));
    query.bindValue(":qsosu_operator_id", getCallsignID(qso_data.at(6)));
    query.bindValue(":station_callsign", qso_data.at(5));
    query.bindValue(":operator", qso_data.at(6));
    query.bindValue(":my_gridsquare", qso_data.at(8));
    query.bindValue(":my_cnty", qso_data.at(7));
    query.bindValue(":call", qso_data.at(0));
    query.bindValue(":qso_date", qso_data.at(9));
    query.bindValue(":time_on", qso_data.at(10));
    query.bindValue(":time_off", qso_data.at(11));
    query.bindValue(":band", qso_data.at(3));
    query.bindValue(":freq", 0);
    query.bindValue(":mode", qso_data.at(4));
    query.bindValue(":rst_sent", qso_data.at(12));
    query.bindValue(":rst_rcvd", qso_data.at(13));
    query.bindValue(":name", "");
    query.bindValue(":qth", "");
    query.bindValue(":gridsquare", qso_data.at(2));
    query.bindValue(":cnty", qso_data.at(1));
    query.bindValue(":comment", "");
    query.bindValue(":sync_state", 1);
    query.bindValue(":hash", qso_data.at(14));
    query.bindValue(":ituz", 0);
    query.bindValue(":cqz", 0);
    query.bindValue(":sync_qso", 1);

    if (!query.exec()) qDebug() << "ERROR Insert into database." << query.lastError() << "\n" << qso_data;
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
