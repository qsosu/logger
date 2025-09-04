#ifndef REPORTBANDS_H
#define REPORTBANDS_H

#include <QDialog>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QTableView>
#include <QAbstractItemModel>
#include <QFontMetrics>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFile>

namespace Ui {
class ReportBands;
}

class ReportBands : public QDialog
{
    Q_OBJECT

public:
    explicit ReportBands(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportBands();
    void printBandsReport(QTableView *tableView, QWidget *parent = nullptr);

private slots:
    void on_CloseButton_clicked();
    void on_PrintButton_clicked();

private:
    Ui::ReportBands *ui;
    QSqlDatabase db;
};

#endif // REPORTBANDS_H
