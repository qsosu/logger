#ifndef REPORTMODES_H
#define REPORTMODES_H

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
class ReportModes;
}

class ReportModes : public QDialog
{
    Q_OBJECT

public:
    explicit ReportModes(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportModes();
    void printModesReport(QTableView *tableView, QWidget *parent = nullptr);

private slots:
    void on_CloseButton_clicked();
    void on_PrintButton_clicked();

private:
    Ui::ReportModes *ui;
    QSqlDatabase db;
};

#endif // REPORTMODES_H
