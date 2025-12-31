#ifndef REPORTYEAR_H
#define REPORTYEAR_H

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
class ReportYear;
}

class ReportYear : public QDialog
{
    Q_OBJECT

public:
    explicit ReportYear(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportYear();
    void printBandsReport(QTableView *tableView, QWidget *parent = nullptr);

private slots:
    void on_CloseButton_clicked();
    void on_PrintButton_clicked();

private:
    Ui::ReportYear *ui;
    QSqlDatabase db;
};

#endif // REPORTYEAR_H
