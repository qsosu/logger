/**********************************************************************************************************
Description :  ImportADIF dialog class for importing QSOs from ADIF (*.adi) files into the database.
Version     :  1.2.0
Date        :  11.07.2025
Author      :  R9JAU
Comments    :  - Automatically matches callsigns against prefix entries to determine country, continent,
            :  and country code.
            :  - Handles TIME_ON, TIME_OFF, QSO_DATE, FREQ, RST, BAND, MODE, NAME, QTH, GRIDSQUARE,
            :  COMMENT, ITUZ, CQZ.
**********************************************************************************************************/

#include "importadif.h"
#include "ui_importadif.h"
#include <QRegularExpression>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

ImportADIF::ImportADIF(QSqlDatabase dbconn, QVector<CountryEntry> entries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportADIF)
{
    ui->setupUi(this);
    db = dbconn;
    this->entries = entries;
    setWindowTitle(tr("Загрузка лога из ADIF  файла"));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->ImportProgressBar->setMinimum(0);
    getCallsigns();
}
//------------------------------------------------------------------------------------------------------------------------------------------

ImportADIF::~ImportADIF()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QList<QsoRecord> ImportADIF::parse(const QString &filePath)
{
    QList<QsoRecord> records;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return records;
    }

    QTextStream in(&file);
    //in.setCodec("UTF-8");
    QString data = in.readAll();

    // Приводим всё к одному виду (удаляем заголовки и символы перехода строк)
    data.replace("\r", " ");
    data.replace("\n", " ");
    //data = data.toLower(); // ADIF нечувствителен к регистру

    // Удаляем заголовки до <EOH> (если есть)
    int eohIndex = data.indexOf("<EOH>");
    if (eohIndex != -1)
        data = data.mid(eohIndex + 5);

    // Разбиваем по <EOR>
    QStringList entries = data.split("<EOR>", Qt::SkipEmptyParts);
    QRegularExpression fieldRegex(R"(<([a-z0-9_]+):(\d+)(:[a-z])?>([^<]*))", QRegularExpression::CaseInsensitiveOption);

    for (const QString &entry : entries) {
        QsoRecord record;
        QRegularExpressionMatchIterator it = fieldRegex.globalMatch(entry);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString field = match.captured(1).toUpper();
            int length = match.captured(2).toInt();
            QString value = match.captured(4).left(length).trimmed();
            record.fields.insert(field, value);
        }
        if (!record.fields.isEmpty())
            records.append(record);
    }
    return records;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ImportADIF::on_importButton_clicked()
{
    QSO qso;
    long cnt, count;
    cnt = 0;

    if(fileName.length() < 4) {
        QMessageBox::critical(0, tr("Ошибка"), tr("Не выбран файл"), QMessageBox::Ok);
        return;
    }
    QList<QsoRecord> records = parse(fileName);

    QApplication::processEvents();
    count = records.count();
    ui->totalQSOLabel->setText("Всего QSO: " + QString::number(count));
    ui->ImportProgressBar->setMinimum(0);
    ui->ImportProgressBar->setMaximum(records.count());

    for (const auto &rec : records)
    {
        qso.stationCallsign = rec.fields.value("STATION_CALLSIGN");
        qso.oper = rec.fields.value("OPERATOR");
        qso.call = rec.fields.value("CALL");
        qso.name = rec.fields.value("NAME");
        qso.city = rec.fields.value("QTH");
        qso.my_gridsquare = rec.fields.value("MY_GRIDSQUARE");
        qso.my_cnty = rec.fields.value("MY_CNTY");
        qso.gridsquare = rec.fields.value("GRIDSQUARE");
        qso.qsoDate = rec.fields.value("QSO_DATE");
        qso.qsoDateOff = rec.fields.value("QSO_DATE_OFF");
        qso.timeOn = rec.fields.value("TIME_ON");
        qso.timeOff = rec.fields.value("TIME_OFF");
        qso.mode = rec.fields.value("MODE");
        qso.rstSent = rec.fields.value("RST_SENT");
        qso.rstRcvd = rec.fields.value("RST_RCVD");
        qso.band = rec.fields.value("BAND");
        qso.freq = rec.fields.value("FREQ");
        qso.cqz = rec.fields.value("CQZ");
        qso.ituz = rec.fields.value("ITUZ");
        //qso.country = rec.fields.value("COUNTRY");
        //qso.continent = rec.fields.value("CONT");
        qso.comment = rec.fields.value("QSLMSG");

        CountryEntry result = findCountryByCall(qso.call, entries);
        if(!result.country.isEmpty())
        {
            qso.country = result.country;
            qso.country_code = countryToIso.value(result.country, "");
            qso.continent = result.continent;
        } else {
            qso.country = "";
            qso.country_code = "";
            qso.continent = "";
            qDebug() << "Страна не определена: " << qso.call;
        }

        InsertQso(qso);
        ui->QSOCountLabel->setText("Загружено QSO: " + QString::number(++cnt));
        ui->ImportProgressBar->setValue(cnt);
        QApplication::processEvents();
    }

    QSqlQuery cpy_query(db);
    cpy_query.prepare("INSERT  OR IGNORE INTO RECORDS (callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, "
                  "TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT, sync_state, HASH, ITUZ, CQZ, COUNTRY, COUNTRY_CODE, CONT, SYNC_QSO) "
                  "SELECT callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, "
                  "TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT, sync_state, HASH, ITUZ, CQZ, COUNTRY, COUNTRY_CODE, CONT, SYNC_QSO "
                  "FROM TEMP_RECORDS ORDER BY QSO_DATE");
    if(!cpy_query.exec()) qDebug() << "ERROR copy data from temp_records into database." << cpy_query.lastError() << "\n";
    cpy_query.prepare("DELETE FROM TEMP_RECORDS");
    if(!cpy_query.exec()) qDebug() << "ERROR CLEAR TEMP_RECORDS TABLE. " << cpy_query.lastError() << "\n";
    emit db_updated();
    QMessageBox::information(this, tr("Загрузка QSO из ADIF файла"), tr("Загружено QSO: ") + QString::number(cnt));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ImportADIF::InsertQso(const QSO &qso)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO TEMP_RECORDS (callsign_id, qsosu_callsign_id, qsosu_operator_id, STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF,"
                  "BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COUNTRY_CODE, COUNTRY, CONT, COMMENT, sync_state, HASH, ITUZ, CQZ, COUNTRY, CONT, SYNC_QSO, QSL_STATUS) "
                  "VALUES (:callsign_id, :qsosu_callsign_id, :qsosu_operator_id, :station_callsign, :operator, :my_gridsquare, :my_cnty, :call, :qso_date, :time_on, :time_off, :band, :freq, "
                  ":mode, :rst_sent, :rst_rcvd, :name, :qth, :gridsquare, :cnty, :country_code, :country, :cont, :comment, :sync_state, :hash, :ituz, :cqz, :country, :cont, :sync_qso, :qsl_status)");
    query.bindValue(":callsign_id", getLocalCallsignID(ui->OperatorComboBox->currentText()));
    query.bindValue(":qsosu_callsign_id", getCallsignID(ui->OperatorComboBox->currentText()));
    query.bindValue(":qsosu_operator_id", getCallsignID(qso.oper));
    query.bindValue(":station_callsign", qso.call.toUpper());
    query.bindValue(":operator", qso.oper.toUpper());
    query.bindValue(":my_gridsquare", qso.my_gridsquare.toUpper());
    query.bindValue(":my_cnty", qso.my_cnty.toUpper());
    query.bindValue(":call", qso.call.toUpper());
    query.bindValue(":qso_date", qso.qsoDate);
    query.bindValue(":time_on", qso.timeOn);
    query.bindValue(":time_off", qso.timeOff);
    query.bindValue(":band", qso.band);
    unsigned long long freqHz = static_cast<unsigned long long>(qso.freq.toDouble() * 1000000);
    query.bindValue(":freq", freqHz);
    query.bindValue(":mode", qso.mode.toUpper());
    query.bindValue(":rst_sent", qso.rstSent);
    query.bindValue(":rst_rcvd", qso.rstRcvd);
    query.bindValue(":name", qso.name);
    query.bindValue(":qth", qso.city);
    query.bindValue(":gridsquare", qso.gridsquare.toUpper());
    query.bindValue(":cnty", "");
    query.bindValue(":comment", qso.comment);
    query.bindValue(":sync_state", 0);
    query.bindValue(":hash", QVariant(QVariant::String));
    query.bindValue(":ituz", qso.ituz);
    query.bindValue(":cqz", qso.cqz);
    query.bindValue(":country", qso.country);
    query.bindValue(":country_code", qso.country_code);
    query.bindValue(":cont", qso.continent.toUpper());
    query.bindValue(":sync_qso", 0);
    query.bindValue(":qsl_status", 0);

    if (!query.exec()) qDebug() << "ERROR Insert into database." << query.lastError() << "\n";
}
//------------------------------------------------------------------------------------------------------------------------------------------


int ImportADIF::getLocalCallsignID(QString callsign)
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

int ImportADIF::getCallsignID(QString callsign)
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

void ImportADIF::on_closeButton_clicked()
{
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ImportADIF::on_pushButton_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, ("Открытие файла"), "", ("ADIF (*.adi)"));
    ui->lineEdit->setText(fileName);

}
//------------------------------------------------------------------------------------------------------------------------------------------

void ImportADIF::getCallsigns() {
  ui->CallSignComboBox->clear();
  ui->OperatorComboBox->clear();

  QSqlQuery query(db);
  query.exec("SELECT id, qsosu_id, type, name, gridsquare, cnty FROM callsigns");
  while(query.next()) {
      int id = query.value(0).toInt();
      int qsosu_id = query.value(1).toInt();
      int type = query.value(2).toInt();
      QString name = query.value(3).toString();
      QString gridsquare = query.value(4).toString();
      QString cnty = query.value(5).toString();
      ui->CallSignComboBox->addItem(name, QList<QVariant>() << id << qsosu_id << type << gridsquare << cnty);
      if(type == 0) ui->OperatorComboBox->addItem(name, QList<QVariant>() << id << qsosu_id); //Bug Fix Change
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

CountryEntry ImportADIF::findCountryByCall(const QString &call, const QVector<CountryEntry> &cty)
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
