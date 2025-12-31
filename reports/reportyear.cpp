/**********************************************************************************************************
Description :  ReportYear dialog class for displaying QSO statistics by year.
Version     :  1.0.0
Date        :  09.09.2025
Author      :  R9JAU
Comments    :  - Supports printing the table to a printer with automatic page layout.
               - Column widths are calculated dynamically and the first column expands if extra space is available.
**********************************************************************************************************/

#include "reportyear.h"
#include "ui_reportyear.h"
#include <QSqlError>
#include <QPrinterInfo>
#include <QDebug>
#include <QMessageBox>
#include <QDate>


ReportYear::ReportYear(QSqlDatabase db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportYear)
{
    ui->setupUi(this);
    this->db = db;
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels({tr("Год"), tr("Количество QSO")});

    db.commit();
    QSqlQuery query(db);
    int row = 0;

    if (query.exec("SELECT SUBSTR(QSO_DATE, 1, 4) AS Year, COUNT(*) AS QSO_Count FROM records GROUP BY Year ORDER BY Year;"))
    {
        while (query.next()) {
            QString year = query.value(0).toString();
            int count = query.value(1).toInt();

            model->setItem(row, 0, new QStandardItem(year));
            model->setItem(row, 1, new QStandardItem(QString::number(count)));
            row++;
        }
    }
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
//--------------------------------------------------------------------------------------------------------------------

ReportYear::~ReportYear()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------

void ReportYear::on_CloseButton_clicked()
{
    close();
}
//--------------------------------------------------------------------------------------------------------------------

void ReportYear::printBandsReport(QTableView *tableView, QWidget *parent)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(false);
    printer.setResolution(300);

    QPrintDialog dialog(&printer, parent);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    if (!painter.isActive())
        return;

    QRect pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
    int leftMargin = 50;
    int rightMargin = 50;
    int topMargin = 150;
    int bottomMargin = 130;
    int headerSpacing = 40;
    int rowHeight = 85;

    int usableWidth = pageRect.width() - leftMargin - rightMargin;
    int usableHeight = pageRect.height() - topMargin - bottomMargin - headerSpacing - 100;

    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QFontMetrics fm(font);

    QAbstractItemModel* model = tableView->model();
    if (!model) return;

    int totalRows = model->rowCount();
    if (model->columnCount() < 2) return;

    // Ширина колонки с номерами строк
    QString maxRowNumberStr = QString::number(totalRows);
    int rowNumberWidth = fm.horizontalAdvance(maxRowNumberStr) + 20;

    // Вычисляем ширину YEAR
    int colBandWidth = fm.horizontalAdvance(model->headerData(0, Qt::Horizontal).toString()) + 20;
    for (int row = 0; row < totalRows; ++row) {
        QString band = model->index(row, 0).data().toString().toUpper();
        colBandWidth = std::max(colBandWidth, fm.horizontalAdvance(band) + 20);
    }

    // Вычисляем ширину COUNT
    int colCountWidth = fm.horizontalAdvance(model->headerData(1, Qt::Horizontal).toString()) + 20;
    for (int row = 0; row < totalRows; ++row) {
        QString countStr = model->index(row, 1).data().toString();
        colCountWidth = std::max(colCountWidth, fm.horizontalAdvance(countStr) + 20);
    }

    // Если осталось место — увеличим первую колонку
    int totalWidth = rowNumberWidth + colBandWidth + colCountWidth;
    if (totalWidth < usableWidth) {
        colBandWidth += usableWidth - totalWidth;
    }

    int rowsPerPage = usableHeight / rowHeight;
    int page = 1;

    for (int startRow = 0; startRow < totalRows; startRow += rowsPerPage) {
        if (page > 1) printer.newPage();

        // Заголовок
        QFont titleFont = painter.font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(pageRect.left() + leftMargin + 100, pageRect.top() + 130, tr("Статистика по годам"));
        painter.setFont(font);

        int x = pageRect.left() + leftMargin;
        int y = pageRect.top() + topMargin;

        // Заголовки
        painter.drawRect(x, y, rowNumberWidth, rowHeight);
        painter.drawText(QRect(x + 5, y, rowNumberWidth - 10, rowHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, "#");
        x += rowNumberWidth;

        painter.drawRect(x, y, colBandWidth, rowHeight);
        painter.drawText(QRect(x + 5, y, colBandWidth - 10, rowHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, tr("Годы"));
        x += colBandWidth;

        painter.drawRect(x, y, colCountWidth, rowHeight);
        painter.drawText(QRect(x + 5, y, colCountWidth - 10, rowHeight),
                         Qt::AlignVCenter | Qt::AlignLeft, tr("Кол-во"));
        y += rowHeight;

        // Данные
        int rowsOnPage = std::min(rowsPerPage, totalRows - startRow);
        for (int i = 0; i < rowsOnPage; ++i) {
            x = pageRect.left() + leftMargin;
            int row = startRow + i;

            // № строки
            painter.drawRect(x, y, rowNumberWidth, rowHeight);
            painter.drawText(QRect(x + 5, y, rowNumberWidth - 10, rowHeight),
                             Qt::AlignVCenter | Qt::AlignLeft, QString::number(row + 1));
            x += rowNumberWidth;

            // YEAR (всегда верхний регистр)
            QString band = model->index(row, 0).data().toString().toUpper();
            painter.drawRect(x, y, colBandWidth, rowHeight);
            painter.drawText(QRect(x + 5, y, colBandWidth - 10, rowHeight), Qt::AlignVCenter | Qt::AlignLeft, band);
            x += colBandWidth;

            // COUNT
            QString countStr = model->index(row, 1).data().toString();
            painter.drawRect(x, y, colCountWidth, rowHeight);
            painter.drawText(QRect(x + 5, y, colCountWidth - 10, rowHeight),
                             Qt::AlignVCenter | Qt::AlignLeft, countStr);
            y += rowHeight;
        }

        // Футер
        QString footer = QString(tr("Страница %1    Дата: %2"))
                .arg(page)
                .arg(QDate::currentDate().toString("dd.MM.yyyy"));
        QFont footerFont = font;
        footerFont.setPointSize(9);
        painter.setFont(footerFont);
        painter.drawText(pageRect.left() + leftMargin,
                         pageRect.top() + pageRect.height() - bottomMargin + 30,
                         footer);
        page++;
    }
    painter.end();
}
//--------------------------------------------------------------------------------------------------------------------

void ReportYear::on_PrintButton_clicked()
{
    printBandsReport(ui->tableView, this);
}
//--------------------------------------------------------------------------------------------------------------------
