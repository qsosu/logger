/*******************************************************************************************************************
Description :  UploadingLogs dialog class for downloading and inserting QSO logs from QSO.SU into local database.
               Supports paging, temporary storage in TEMP_RECORDS, frequency normalization, prefix/DXCC resolution,
               and progress tracking in UI.
Version     :  1.0.0
Date        :  12.08.2025
Author      :  R9JAU
Comments    :  - Uses HttpApi to fetch QSO logs.
               - Supports country/continent/CQ/ITU zone resolution using PrefixEntry list.
               - Caches callsign IDs for faster insertion.
********************************************************************************************************************/

#include "uploadinglogs.h"
#include "ui_uploadinglogs.h"
#include <QSqlError>


UploadingLogs::UploadingLogs(QSqlDatabase db, Settings *settings, QVector<CountryEntry> entries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadingLogs)
{
    this->db = db;
    this->settings = settings;
    this->entries = entries;
    ui->setupUi(this);
    qso_count = 0;


    setWindowTitle(tr("Загрузка лога с QSO.SU"));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    api = new HttpApi(db, settings->accessToken);
    connect(api, SIGNAL(uploadQSOs(int)), this, SLOT(UploadQSOs(int)));
    getCallsigns();

    ui->UploadProgressBar->setValue(0);
}
//------------------------------------------------------------------------------------------------------------------------------------------

UploadingLogs::~UploadingLogs()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UploadingLogs::on_CloseCloseButton_clicked()
{
    ui->UploadProgressBar->setValue(0);
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UploadingLogs::UploadQSOs(int count)
{
    ui->QSOCountLabel->setText(tr("Всего QSO: ") + QString::number(count));
    totalCount = count;
    bool confirmation = false;
    QString hash;

    if(totalCount == 0) {
        ui->UploadProgressBar->setValue(totalCount);
        ui->CurrentQSOCountLabel->setText(tr("Загружено QSO: ") + QString::number(totalCount));
        QMessageBox::information(0, tr("Загрузка QSO"), tr("Нет QSO для загрузки."), QMessageBox::Ok);
        return;
    }

    ui->UploadProgressBar->setMaximum(count);
    dbid = getMaxID();

    QSqlQuery query(db);
    query.prepare("INSERT INTO TEMP_RECORDS "
                  "(callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF, "
                  "BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COUNTRY_CODE, COUNTRY, CONT, COMMENT, sync_state, HASH, ITUZ, CQZ, SYNC_QSO) "
                  "VALUES "
                  "(:callsign_id, :qsosu_callsign_id, :qsosu_operator_id, :station_callsign, :operator, :my_gridsquare, :my_cnty, :call, :qso_date, :time_on, :time_off, "
                  ":band, :freq, :mode, :rst_sent, :rst_rcvd, :name, :qth, :gridsquare, :cnty, :country_code, :country, :cont, :comment, :sync_state, :hash, :ituz, :cqz, :sync_qso)");

    db.transaction(); // Начинаем транзакцию

    for (QVariantMap &uploadQSO : api->uploadLogs)
    {
        QString band = uploadQSO["BAND"].toString();
        QString call = uploadQSO["CALL"].toString();
        QString cnty = uploadQSO["CNTY"].toString();
        QString cont = uploadQSO["CONT"].toString();
        QString country = uploadQSO["COUNTRY"].toString();
        int cqz = uploadQSO["CQZ"].toInt();
        double freq = uploadQSO["FREQ"].toDouble();
        unsigned long long freqHz = static_cast<unsigned long long>(freq * 1000000);
        QString gridsquare = uploadQSO["GRIDSQUARE"].toString();
        int ituz = uploadQSO["ITUZ"].toInt();
        QString mode = uploadQSO["MODE"].toString();
        QString my_cnty = uploadQSO["MY_CNTY"].toString();
        QString my_gridsquare = uploadQSO["MY_GRIDSQUARE"].toString();
        QString name = uploadQSO["NAME"].toString();
        QString st_oper = uploadQSO["OPERATOR"].toString();
        QString pfx = uploadQSO["PFX"].toString();
        QString qso_date = uploadQSO["QSO_DATE"].toString();
        QString qso_date_off = uploadQSO["QSO_DATE_OFF"].toString();
        QString qth = uploadQSO["QTH"].toString();
        QString rst_rcvd = uploadQSO["RST_RCVD"].toString();
        QString rst_sent = uploadQSO["RST_SENT"].toString();
        QString state = uploadQSO["STATE"].toString();
        QString station_callsign = uploadQSO["STATION_CALLSIGN"].toString();
        QString submode = uploadQSO["SUBMODE"].toString();
        QString time_off = uploadQSO["TIME_OFF"].toString();
        QString time_on = uploadQSO["TIME_ON"].toString();

        QVariant extraVar = uploadQSO.value("extra");
        if (extraVar.isValid() && extraVar.type() == QVariant::Map) {
            QVariantMap extraMap = extraVar.toMap();
            confirmation = extraMap.value("confirmation").toBool();
            hash = extraMap.value("hash").toString();
        }

        QString country_code = "";

        CountryEntry result = findCountryByCall(call, entries);
        if(!result.country.isEmpty())
        {
            country = result.country;
            country_code = countryToIso.value(result.country, "");
            cont = result.continent;
        } else {
            country = "";
            country_code = "";
            cont = "";
            qDebug() << "Страна не определена: " << call;
        }

        // Биндим значения
        query.bindValue(":callsign_id", callsignToLocalID.value(station_callsign, 0));
        query.bindValue(":qsosu_callsign_id", callsignToID.value(station_callsign, 0));
        query.bindValue(":qsosu_operator_id", callsignToID.value(st_oper, 0));
        query.bindValue(":station_callsign", station_callsign);
        query.bindValue(":operator", st_oper);
        query.bindValue(":my_gridsquare", my_gridsquare);
        query.bindValue(":my_cnty", my_cnty);
        query.bindValue(":call", call);
        query.bindValue(":qso_date", qso_date);
        query.bindValue(":time_on", time_on);
        query.bindValue(":time_off", time_off);
        query.bindValue(":band", band);
        query.bindValue(":freq", freqHz);
        query.bindValue(":mode", mode);
        query.bindValue(":rst_sent", rst_sent);
        query.bindValue(":rst_rcvd", rst_rcvd);
        query.bindValue(":name", name);
        query.bindValue(":qth", qth);
        query.bindValue(":gridsquare", gridsquare);
        query.bindValue(":cnty", cnty);
        query.bindValue(":country_code", country_code);
        query.bindValue(":country", country);
        query.bindValue(":cont", cont);
        query.bindValue(":comment", "");
        query.bindValue(":sync_state", confirmation);
        query.bindValue(":hash", hash);
        query.bindValue(":ituz", ituz);
        query.bindValue(":cqz", cqz);
        query.bindValue(":sync_qso", 3);

        if (!query.exec()) {
            qDebug() << "ERROR Insert into database." << query.lastError() << "\n";
        }

        qso_count++;
        if(qso_count % 50 == 0 || qso_count == count) {
            ui->UploadProgressBar->setValue(qso_count);
            ui->CurrentQSOCountLabel->setText(tr("Загружено QSO: ") + QString::number(qso_count));
            QApplication::processEvents();
        }
    }

    db.commit(); // Завершаем транзакцию
    qDebug() << "Uploaded: " << qso_count << " QSOs";
    emit db_updated();
}
//------------------------------------------------------------------------------------------------------------------------------------------

int UploadingLogs::getMaxID()
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

void UploadingLogs::getCashCallsignID()
{
    QSqlQuery q(db);
    q.exec("SELECT name, qsosu_id, id FROM callsigns");
    while(q.next()) {
       QString name = q.value(0).toString();
       callsignToID[name] = q.value(1).toInt();
       callsignToLocalID[name] = q.value(2).toInt();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UploadingLogs::getCallsigns()
{
    int type;
    ui->CallSignComboBox->clear();
    ui->OperatorComboBox->clear();

    QSqlQuery query(db);
    query.exec("SELECT type, name FROM callsigns");
    while(query.next()) {
        type = query.value(0).toInt();
        QString name = query.value(1).toString();
        ui->CallSignComboBox->addItem(name);
        if(type == 0) ui->OperatorComboBox->addItem(name);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

int UploadingLogs::getCallsignID(QString callsign)
{
    int value = 0;
    QSqlQuery query(db);

    query.exec(QString("SELECT qsosu_id FROM callsigns WHERE name = '%1'").arg(callsign));
    while(query.next())
    {
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

int UploadingLogs::getLocalCallsignID(QString callsign)
{
    int value = 0;

    QSqlQuery query(db);
    query.exec(QString("SELECT id FROM callsigns WHERE name = '%1'").arg(callsign));

    while(query.next())
    {
        value = query.value(0).toInt();
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UploadingLogs::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void UploadingLogs::on_UploadButton_clicked()
{
    int pages;
    int page = 1; //Номер страницы (по умолчанию 1)
    int page_count = 1000; //Количество записей на страницу (100–5000)

    qso_count = 0;

    getCashCallsignID();
    int station_id = getCallsignID(ui->CallSignComboBox->currentText());
    int operator_id = getCallsignID(ui->OperatorComboBox->currentText());

    api->getLogs(operator_id, station_id, page, page_count);

    QEventLoop loop;
    QObject::connect(api, &HttpApi::uploadQSOs, &loop, &QEventLoop::quit);
    loop.exec();

    pages = static_cast<int>(qCeil((double)totalCount / page_count));

    for(int j = 2; j <= pages; j++)
    {
       api->getLogs(operator_id, station_id, j, page_count);
       QObject::connect(api, &HttpApi::uploadQSOs, &loop, &QEventLoop::quit);
       loop.exec();
    }

    db.transaction();
    QSqlQuery cpy(db);
    if(!cpy.exec("INSERT OR IGNORE INTO RECORDS (callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, "
                 "TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COUNTRY_CODE, COUNTRY, CONT, COMMENT, sync_state, HASH, ITUZ, CQZ, SYNC_QSO) "
                 "SELECT callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, "
                 "TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COUNTRY_CODE, COUNTRY, CONT, COMMENT, sync_state, HASH, ITUZ, CQZ, SYNC_QSO "
                 "FROM TEMP_RECORDS ORDER BY QSO_DATE")) {
        qDebug() << "ERROR copy TEMP_RECORDS -> RECORDS:" << cpy.lastError();
    }

    if(!cpy.exec("DELETE FROM TEMP_RECORDS")) {
        qDebug() << "ERROR clearing TEMP_RECORDS:" << cpy.lastError();
    }
    db.commit();
    emit db_updated();
    QMessageBox::information(this, tr("Загрузка лога с QSO.SU"), tr("Загружено QSO: ") + QString::number(qso_count));
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool UploadingLogs::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        getCallsigns();
    }
    return QDialog::event(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString UploadingLogs::normalizeFreq(const QString &freqStr, const QString &band)
{
    struct HamBand { QString name; qint64 lo; qint64 hi; };
    static const HamBand kBands[] = {
        {"160M",  1800000,  2000000},
        {"80M",   3500000,  4000000},
        {"60M",   5250000,  5450000},
        {"40M",   7000000,  7300000},
        {"30M",  10100000,10150000},
        {"20M",  14000000,14350000},
        {"17M",  18000000,18168000},
        {"15M",  21000000,21450000},
        {"12M",  24000000,24990000},
        {"10M",  28000000,29700000},
        {"6M",   50000000,  52000000},
        {"4M",   70000000,  71000000},
        {"2M",  144000000,148000000},
        {"70CM",430000000,440000000},
        {"23CM",1240000000,1300000000}
    };

    // ищем диапазон
    qint64 lo=0, hi=0;
    for (const auto &b : kBands) {
        if (b.name.compare(band, Qt::CaseInsensitive) == 0) {
            lo = b.lo;
            hi = b.hi;
            break;
        }
    }
    if (lo==0 && hi==0) return "0"; // неизвестный диапазон

    QString s = freqStr.trimmed();
    s.replace(',', '.');

    bool ok=false;
    double val = s.toDouble(&ok);
    if (!ok) return "0";

    // если дробь — трактуем как MHz
    if (s.contains('.')) {
        qint64 hz = static_cast<qint64>(val * 1'000'000.0 + 0.5);
        return QString::number(hz);
    }

    qint64 intVal = static_cast<qint64>(val);

    // корректировка множителя
    qint64 hz = intVal;
    qint64 multiplier = 1;

    while (hz < lo) {
        multiplier *= 10;
        hz = intVal * multiplier;
    }

    // если получилась частота выше диапазона hi, делим на 10
    if (hz > hi) {
        multiplier /= 10;
        hz = intVal * multiplier;
    }

    return QString::number(hz);
}
//------------------------------------------------------------------------------------------------------------------------------------------

CountryEntry UploadingLogs::findCountryByCall(const QString &call, const QVector<CountryEntry> &cty)
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
