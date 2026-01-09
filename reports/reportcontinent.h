#ifndef REPORTCONTINENT_H
#define REPORTCONTINENT_H

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
class ReportContinent;
}

class ReportContinent : public QDialog
{
    Q_OBJECT

public:
    explicit ReportContinent(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportContinent();
    void printTableAdvanced(QTableView *tableView, QWidget *parent = nullptr);


private slots:
    void on_PrintButton_clicked();
    void on_CloseButton_clicked();

private:
    Ui::ReportContinent *ui;
    QSqlDatabase db;
};

#endif // REPORTCONTINENT_H
