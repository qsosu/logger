/**********************************************************************************************************
Description :  ExportADIF dialog class for exporting QSO records to ADIF files.
Version     :  1.0.0
Date        :  11.07.2025
Author      :  R9JAU
Comments    :  - Allows exporting QSO data for selected station/operator to ADIF format.
               - Supports exporting all QSOs or QSOs within a date range.
               - Automatically formats QSO_DATE, TIME_ON, TIME_OFF, FREQ according to ADIF 3.1.0 standard.
**********************************************************************************************************/

#include "exportadif.h"
#include "ui_exportadif.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

ExportADIF::ExportADIF(QSqlDatabase dbconn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportADIF)
{
    ui->setupUi(this);
    db = dbconn;
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QDateTime date_now = QDateTime::currentDateTime();
    ui->FromDateEdit->setDateTime(date_now);
    ui->ToDateEdit->setDateTime(date_now);
    getCallsigns();
}
//------------------------------------------------------------------------------------------------------------------------------------------

ExportADIF::~ExportADIF()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::ExportAll(QString callsign, QString oper)
{
    QSqlQuery query(db);

    query.prepare("SELECT STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT FROM records "
                  "WHERE STATION_CALLSIGN=:call AND OPERATOR=:operator");
    query.bindValue(":call", callsign);
    query.bindValue(":operator", oper);
    if (!query.exec()) {
        qDebug() << "ERROR SELECT FROM DATABASE. " << query.lastError().text();
    }
    WriteFile(std::move(query), "ALL");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::Export(QString callsign, QString oper, QString data_from, QString data_to)
{
    QSqlQuery query(db);

    qDebug() << callsign << " " << oper << " " << data_from << " " << data_to;

    query.prepare("SELECT STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT, CQZ, ITUZ, CONT FROM records "
                  "WHERE STATION_CALLSIGN=:call AND OPERATOR=:operator AND QSO_DATE >= :dfrom AND QSO_DATE <= :dto");
    query.bindValue(":call", callsign);
    query.bindValue(":operator", oper);
    query.bindValue(":dfrom", data_from);
    query.bindValue(":dto", data_to);
    if (!query.exec()) {
        qDebug() << "ERROR SELECT FROM DATABASE. " << query.lastError().text();
    }
    WriteFile(std::move(query), "FROM DATE");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::WriteFile(QSqlQuery query, QString suffix)
{
    QDateTime export_date = QDateTime::currentDateTime();
    QString export_date_formated = export_date.toString("yyyyMMdd_hhmmss");
    QString fileName = QString("/%1_%2_QSOSU_export.adi").arg(export_date_formated, suffix);

    QString outFile = QFileDialog::getSaveFileName(0, "Сохранить ADIF файл...", QDir::homePath() + fileName, "*.adi");
    if (outFile.length() == 0) return;

    QFile ADIFFile(outFile);
    int counter = 0;

    if (ADIFFile.open(QIODevice::ReadWrite)) {
        QTextStream ADIFLine(&ADIFFile);
        ADIFLine << "<PROGRAMID:19>QSOLogger" << Qt::endl;
        ADIFLine << "<PROGRAMVERSION:3>3.0" << Qt::endl;
        ADIFLine << "<ADIF_VER:3>2.0" << Qt::endl;
        ADIFLine << "<EOH>" << Qt::endl;

        while (query.next()) {
            QString station_callsign = query.value(0).toString();
            QString op = query.value(1).toString();
            QString my_gridsquare = query.value(2).toString();
            QString my_cnty = query.value(3).toString();
            QString call = query.value(4).toString();
            QString qso_date = query.value(5).toString().remove("-");
            QString time_on = query.value(6).toString().remove(":").left(4);
            QString band = query.value(8).toString();
            QString freq = QString::number((double) query.value(9).toInt() / 1000000);
            double f = freq.toDouble();
            QString freq3 = QString::number(f, 'f', 3);
            QString mode = query.value(10).toString();
            QString rst_sent = query.value(11).toString();
            QString rst_rcvd = query.value(12).toString();
            QString name = query.value(13).toString();
            QString qth = query.value(14).toString();
            QString gridsquare = query.value(15).toString();
            QString cnty = query.value(16).toString();
            QString comment = query.value(17).toString();
            QString cqz = query.value(18).toString();
            QString ituz = query.value(19).toString();
            QString cont = query.value(20).toString();

            QTextStream QSOLine(&ADIFFile);
            QSOLine.setCodec("Windows-1251");
            QSOLine.setGenerateByteOrderMark(false);
            QSOLine << Qt::endl;
            QSOLine.flush();

            const QString EOL = "\r\n";
            QSOLine << QStringLiteral("<OPERATOR:%1>%2").arg(op.length()).arg(op);
            QSOLine << QStringLiteral("<CALL:%1>%2").arg(call.length()).arg(call);
            QSOLine << QStringLiteral("<QSO_DATE:%1>%2").arg(qso_date.length()).arg(qso_date);
            QSOLine << QStringLiteral("<TIME_ON:%1>%2").arg(time_on.length()).arg(time_on);
            QSOLine << QStringLiteral("<FREQ:%1>%2").arg(freq3.length()).arg(freq3);
            QSOLine << QStringLiteral("<MODE:%1>%2").arg(mode.length()).arg(mode);

            QString rstSentClean = rst_sent;
            rstSentClean.remove(QRegularExpression("[^0-9]"));
            QString rstRcvdClean = rst_rcvd;
            rstRcvdClean.remove(QRegularExpression("[^0-9]"));
            QSOLine << QStringLiteral("<RST_SENT:%1>%2").arg(rstSentClean.length()).arg(rstSentClean);
            QSOLine << QStringLiteral("<STX:0>");
            QSOLine << QStringLiteral("<RST_RCVD:%1>%2").arg(rstRcvdClean.length()).arg(rstRcvdClean);
            QSOLine << QStringLiteral("<SRX:0>");
            if(gridsquare.length() > 0) QSOLine << QStringLiteral("<GRIDSQUARE:%1>%2").arg(gridsquare.length()).arg(gridsquare);
            //<PFX:>
            //<DXCC_PREF:>
            QSOLine << QStringLiteral("<CQZ:%1>%2").arg(cqz.length()).arg(cqz);
            QSOLine << QStringLiteral("<ITUZ:%1>%2").arg(ituz.length()).arg(ituz);
            QSOLine << QStringLiteral("<BAND:%1>%2").arg(band.length()).arg(band);
            QSOLine << QStringLiteral("<CONT:%1>%2").arg(cont.length()).arg(cont);

            QSOLine << QStringLiteral("<NAME:%1>%2").arg(name.length()).arg(name);
            QSOLine << QStringLiteral("<QTH:%1>%2").arg(qth.length()).arg(qth);
            QSOLine << QStringLiteral("<CNTY:%1>%2").arg(cnty.length()).arg(cnty);
            if (comment.length() > 0) {
                QSOLine << QStringLiteral("<QSLMSG:%1>%2").arg(comment.length()).arg(comment);
            }
            //<DXCC:>
            QSOLine << "<EOR>" << EOL;
            counter++;
        }
        QString outInfo;
        outInfo = QString("Создан файл - %1\nQSO в файле - %2").arg(outFile).arg(counter);
        QMessageBox::information(0, "Информация", outInfo);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::getCallsigns()
{
  int type;
  ui->StationComboBox->clear();
  ui->OperatorComboBox->clear();

  QSqlQuery query(db);
  query.exec("SELECT type, name FROM callsigns");
  while(query.next()) {
      type = query.value(0).toInt();
      QString name = query.value(1).toString();
      ui->StationComboBox->addItem(name);
      if(type == 0) ui->OperatorComboBox->addItem(name);
  }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::on_BetweenRadioButton_toggled(bool checked)
{
    if(checked) {
        ui->AllRadioButton->setChecked(false);
        ui->FromDateEdit->setEnabled(true);
        ui->ToDateEdit->setEnabled(true);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::on_AllRadioButton_toggled(bool checked)
{
    if(checked) {
        ui->BetweenRadioButton->setChecked(false);
        ui->FromDateEdit->setEnabled(false);
        ui->ToDateEdit->setEnabled(false);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::on_pushButton_clicked()
{
    close();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ExportADIF::on_ExportButton_clicked()
{
    QString callsign = ui->StationComboBox->currentText();
    QString oper = ui->OperatorComboBox->currentText();
    QString data_from = ui->FromDateEdit->date().toString("yyyyMMdd");
    QString data_to = ui->ToDateEdit->date().toString("yyyyMMdd");

    if(ui->AllRadioButton->isChecked()) {
        ExportAll(callsign, oper);
    } else
        Export(callsign, oper, data_from, data_to);
}
//--------------------------------------------------------------------------------------------------------------------
