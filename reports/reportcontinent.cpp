/**********************************************************************************************************
Description :  ReportContinent dialog class for displaying and printing a summary of QSO counts per continent.
Version     :  1.0.0
Date        :  20.08.2025
Author      :  R9JAU
Comments    :  - Supports printing with headers, footers, and pagination.
               - Automatically adjusts column widths based on content.
**********************************************************************************************************/

#include "reportcontinent.h"
#include "ui_reportcontinent.h"
#include <QSqlError>
#include <QPrinterInfo>
#include <QDebug>
#include <QMessageBox>
#include <QDate>


ReportContinent::ReportContinent(QSqlDatabase db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportContinent)
{
    ui->setupUi(this);
    this->db = db;
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels({tr("Континент"), tr("Код континента"), tr("Количество QSO")});

    // Словарь: код континента -> наименование на русском
    QMap<QString, QString> continentNames =
    {
        {"AF", tr("Африка")},
        {"AN", tr("Антарктида")},
        {"AS", tr("Азия")},
        {"EU", tr("Европа")},
        {"NA", tr("Северная Америка")},
        {"OC", tr("Океания")},
        {"SA", tr("Южная Америка")},
        {"",  tr("Не определено")}
    };

    db.commit();
    QSqlQuery query(db);
    int row = 0;

    if (query.exec("SELECT CONT, COUNT(*) FROM records GROUP BY CONT"))
    {
        while (query.next()) {
            QString code = query.value(0).toString();
            int count = query.value(1).toInt();
            QString name = continentNames.value(code, tr("Неизвестно"));

            model->setItem(row, 0, new QStandardItem(name));
            model->setItem(row, 1, new QStandardItem(code));
            model->setItem(row, 2, new QStandardItem(QString::number(count)));
            row++;
        }
    }
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
//--------------------------------------------------------------------------------------------------------------------

ReportContinent::~ReportContinent()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------

void ReportContinent::on_PrintButton_clicked()
{
    printTableAdvanced(ui->tableView, this);
}
//--------------------------------------------------------------------------------------------------------------------

void ReportContinent::on_CloseButton_clicked()
{
    close();
}
//--------------------------------------------------------------------------------------------------------------------

void ReportContinent::printTableAdvanced(QTableView *tableView, QWidget *parent)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(false);
    printer.setResolution(300);

    QPrintDialog dialog(&printer);
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
    int totalCols = model->columnCount();
    if (totalCols == 0) return;

    // Ширина колонки с номерами строк
    QString maxRowNumberStr = QString::number(totalRows);
    int rowNumberWidth = fm.horizontalAdvance(maxRowNumberStr) + 20;

    // Автовычисление ширины для 2 и 3 столбца
    int col1MaxWidth = fm.horizontalAdvance(model->headerData(1, Qt::Horizontal).toString()) + 20;
    int col2MaxWidth = fm.horizontalAdvance(model->headerData(2, Qt::Horizontal).toString()) + 20;

    for (int row = 0; row < totalRows; ++row) {
        col1MaxWidth = std::max(col1MaxWidth, fm.horizontalAdvance(model->index(row, 1).data().toString()) + 20);
        col2MaxWidth = std::max(col2MaxWidth, fm.horizontalAdvance(model->index(row, 2).data().toString()) + 20);
    }

    int col1Width = col1MaxWidth;
    int col2Width = col2MaxWidth;

    // Ширина первого столбца рассчитывается как оставшееся место
    int col0Width = usableWidth - rowNumberWidth - col1Width - col2Width;

    int rowsPerPage = usableHeight / rowHeight;
    int page = 1;

    for (int startRow = 0; startRow < totalRows; startRow += rowsPerPage) {
        if (page > 1) printer.newPage();

        QFont titleFont = painter.font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);

        QString title = tr("Отчет по отработанным континентам");
        painter.drawText(pageRect.left() + leftMargin + 100, pageRect.top() + 130, title);
        painter.setFont(font);

        int x = pageRect.left() + leftMargin;
        int y = pageRect.top() + topMargin;

        // Номер строки
        painter.drawRect(x, y, rowNumberWidth, rowHeight);
        painter.drawText(x + 5, y + rowHeight - 10, "#");
        x += rowNumberWidth;

        // Заголовки
        QList<int> colWidths = {col0Width, col1Width, col2Width};
        for (int col = 0; col < totalCols; ++col) {
            QRect cellRect(x, y, colWidths[col], rowHeight);
            painter.drawRect(cellRect);
            QString header = model->headerData(col, Qt::Horizontal).toString();
            painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter | Qt::AlignLeft, header);
            x += colWidths[col];
        }

        y += rowHeight;
        int rowsOnPage = std::min(rowsPerPage, totalRows - startRow);

        for (int i = 0; i < rowsOnPage; ++i) {
            x = pageRect.left() + leftMargin;
            int row = startRow + i;

            QRect rowNumRect(x, y, rowNumberWidth, rowHeight);
            painter.drawRect(rowNumRect);
            painter.drawText(rowNumRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter, QString::number(row + 1));
            x += rowNumberWidth;

            for (int col = 0; col < totalCols; ++col) {
                QRect cellRect(x, y, colWidths[col], rowHeight);
                painter.drawRect(cellRect);

                QModelIndex cellIndex = model->index(row, col);
                QString text = model->data(cellIndex, Qt::DisplayRole).toString();
                painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
                x += colWidths[col];
            }
            y += rowHeight;
        }

        QString footer = QString(tr("Страница %1    Дата: %2"))
                .arg(page)
                .arg(QDate::currentDate().toString("dd.MM.yyyy"));

        QFont footerFont = font;
        footerFont.setPointSize(9);
        painter.setFont(footerFont);

        int footerY = pageRect.top() + pageRect.height() - bottomMargin + 30;
        int footerX = pageRect.left() + leftMargin;
        painter.drawText(footerX, footerY, footer);
        page++;
    }

    painter.end();
}
//--------------------------------------------------------------------------------------------------------------------


