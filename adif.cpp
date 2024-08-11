#include "adif.h"
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

Adif::Adif(QSqlDatabase dbconn) {
    db = dbconn;
}

void Adif::Export(int callsign_id) {
    QSqlQuery query(db);
    query.exec(QString("SELECT STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT FROM records WHERE callsign_id=%1 ORDER BY id").arg(callsign_id));
    WriteFile(std::move(query), "ALL");
}

void Adif::ExportPartial(QList<int> indexes) {
    QStringList idstrings;
    foreach(int id, indexes) {
        idstrings << QString::number(id);
    }
    QString numberlist = idstrings.join(",");

    QSqlQuery query(db);
    query.exec(QString("SELECT STATION_CALLSIGN, OPERATOR, MY_GRIDSQUARE, MY_CNTY, CALL, QSO_DATE, TIME_ON, TIME_OFF, BAND, FREQ, MODE, RST_SENT, RST_RCVD, NAME, QTH, GRIDSQUARE, CNTY, COMMENT FROM records WHERE id IN (%1) ORDER BY id").arg(numberlist));
    WriteFile(std::move(query), "SELECTED");
}

void Adif::WriteFile(QSqlQuery query, QString suffix) {
    bool ok;
    QString _comment = QInputDialog::getText(0, "Экспорт в ADIF", "Комментарий к QSO, будет отображен в тэге COMMENT (если отсутствует):", QLineEdit::Normal, "QSO.SU export", &ok);
    if (!ok) {
        return;
    }

    QDateTime export_date = QDateTime::currentDateTime();
    QString export_date_formated = export_date.toString("yyyyMMdd_hhmmss");
    QString fileName = QString("/%1_%2_QSOSU_export.adi").arg(export_date_formated, suffix);

    QString outFile = QFileDialog::getSaveFileName(0, "Сохранить ADIF файл...", QDir::homePath() + fileName, ".adi");
    if (outFile.length() == 0) return;

    QFile ADIFFile(outFile);
    int counter = 0;

    if (ADIFFile.open(QIODevice::ReadWrite)) {
        QTextStream ADIFLine(&ADIFFile);
        ADIFLine << "<ADIF_VER:5>3.1.0" << Qt::endl;
        ADIFLine << "<PROGRAMID:19>QSO.SU-client" << Qt::endl;
        ADIFLine << "<PROGRAMVERSION:3>1.0" << Qt::endl;
        ADIFLine << "<EOH>" << Qt::endl;

        while (query.next()) {
            QString station_callsign = query.value(0).toString();
            QString op = query.value(1).toString();
            QString my_gridsquare = query.value(2).toString();
            QString my_cnty = query.value(3).toString();
            QString call = query.value(4).toString();
            QString qso_date = query.value(5).toString().remove("-");
            QString time_on = query.value(6).toString().remove(":");
            QString time_off = query.value(7).toString().remove(":");
            QString band = query.value(8).toString();
            //QString freq = query.value(9).toString();
            QString freq = QString::number((double) query.value(9).toInt() / 1000000);
            QString mode = query.value(10).toString();
            QString rst_sent = query.value(11).toString();
            QString rst_rcvd = query.value(12).toString();
            QString name = query.value(13).toString();
            QString qth = query.value(14).toString();
            QString gridsquare = query.value(15).toString();
            QString cnty = query.value(16).toString();
            QString comment = query.value(17).toString();

            QTextStream QSOLine(&ADIFFile);
            QSOLine << QStringLiteral("<STATION_CALLSIGN:%1>%2").arg(station_callsign.length()).arg(station_callsign);
            QSOLine << QStringLiteral("<OPERATOR:%1>%2").arg(op.length()).arg(op);
            QSOLine << QStringLiteral("<MY_GRIDSQUARE:%1>%2").arg(my_gridsquare.length()).arg(my_gridsquare);
            QSOLine << QStringLiteral("<MY_CNTY:%1>%2").arg(my_cnty.length()).arg(my_cnty);
            QSOLine << QStringLiteral("<CALL:%1>%2").arg(call.length()).arg(call);
            QSOLine << QStringLiteral("<GRIDSQUARE:%1>%2").arg(gridsquare.length()).arg(gridsquare);
            QSOLine << QStringLiteral("<MODE:%1>%2").arg(mode.length()).arg(mode);
            QSOLine << QStringLiteral("<RST_SENT:%1>%2").arg(rst_sent.length()).arg(rst_sent);
            QSOLine << QStringLiteral("<RST_RCVD:%1>%2").arg(rst_rcvd.length()).arg(rst_rcvd);
            QSOLine << QStringLiteral("<QSO_DATE:%1>%2").arg(qso_date.length()).arg(qso_date);
            QSOLine << QStringLiteral("<TIME_ON:%1>%2").arg(time_on.length()).arg(time_on);
            QSOLine << QStringLiteral("<TIME_OFF:%1>%2").arg(time_off.length()).arg(time_off);
            QSOLine << QStringLiteral("<BAND:%1>%2").arg(band.length()).arg(band);
            QSOLine << QStringLiteral("<FREQ:%1>%2").arg(freq.length()).arg(freq);
            QSOLine << QStringLiteral("<NAME:%1>%2").arg(name.length()).arg(name);
            QSOLine << QStringLiteral("<QTH:%1>%2").arg(qth.length()).arg(qth);
            QSOLine << QStringLiteral("<CNTY:%1>%2").arg(cnty.length()).arg(cnty);
            if (comment.length() > 0) {
                QSOLine << QStringLiteral("<COMMENT:%1>%2").arg(comment.length()).arg(comment);
            } else {
                QSOLine << QStringLiteral("<COMMENT:%1>%2").arg(_comment.length()).arg(_comment);
            }
            QSOLine << "<EOR>" << Qt::endl;

            counter++;
        }

        QString outInfo;
        outInfo = QString("Создан файл - %1\nQSO в файле - %2").arg(outFile).arg(counter);
        QMessageBox::information(0, "Информация", outInfo);
    }
}
