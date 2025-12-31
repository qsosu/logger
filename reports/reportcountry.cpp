/**********************************************************************************************************
Description :  ReportCountry dialog class for displaying and printing a summary of QSO counts per country.
Version     :  1.0.0
Date        :  20.08.2025
Author      :  R9JAU
Comments    :  - Supports printing with headers, footers, and pagination.
               - Automatically scales flag images to fit the cell while maintaining aspect ratio.
**********************************************************************************************************/

#include "reportcountry.h"
#include "ui_reportcountry.h"
#include <QSqlError>
#include <QPrinterInfo>
#include <QDebug>
#include <QMessageBox>
#include <QDate>


ReportCountry::ReportCountry(QSqlDatabase db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportCountry)
{
    ui->setupUi(this);
    this->db = db;
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ColorTableModel *model = new ColorTableModel(this);
    model->setHorizontalHeaderLabels({tr("Флаг"), tr("Страна (территория)"), tr("Континент"), tr("Количество QSO")});

    QSqlQuery query(db);

    int row = 0;
    int count = 0;
    QString country_code;
    QString country;
    QString continent;

    // Подсчет общего числа записей
    if (query.exec("SELECT COUNTRY_CODE, COUNTRY, CONT, COUNT(*) FROM records GROUP BY COUNTRY"))
    {
        model->setRowCount(query.record().count());

        while (query.next()) {
            country_code = query.value(0).toString();
            country = query.value(1).toString();
            continent = query.value(2).toString();
            count = query.value(3).toInt();

            model->setItem(row, 0, new QStandardItem(country_code));

            if(country == "") model->setItem(row, 1, new QStandardItem(tr("Не определено")));
            else model->setItem(row, 1, new QStandardItem(country));

            model->setItem(row, 2, new QStandardItem(continent));
            model->setItem(row, 3, new QStandardItem(QString::number(count)));
            row++;
        }
    }
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
//--------------------------------------------------------------------------------------------------------------------

ReportCountry::~ReportCountry()
{
    delete ui;
}
//--------------------------------------------------------------------------------------------------------------------

void ReportCountry::on_CloseButton_clicked()
{
    close();
}
//--------------------------------------------------------------------------------------------------------------------

void ReportCountry::on_PrintButton_clicked()
{
    printTableAdvanced(ui->tableView, this);
}
//--------------------------------------------------------------------------------------------------------------------

void ReportCountry::printTableAdvanced(QTableView *tableView, QWidget *parent)
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

    // Отступы и размеры
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

    QAbstractItemModel* model = tableView->model();
    if (!model) return;

    int totalRows = model->rowCount();
    int totalCols = model->columnCount();
    if (totalCols == 0) return;

    int flagColIndex = 0;     // Столбец с флагами
    int flagColWidth = 200;    // Фиксированная ширина столбца с флагом
    int variableCols = totalCols - 1;

    // Вычисляем ширину колонки с номерами строк
    QString maxRowNumberStr = QString::number(totalRows);
    int rowNumberWidth = painter.fontMetrics().horizontalAdvance(maxRowNumberStr) + 20;

    // Вычисляем ширину остальных колонок
    int baseColWidth = (usableWidth - rowNumberWidth - flagColWidth) / variableCols;
    int rowsPerPage = usableHeight / rowHeight;
    int page = 1;

    for (int startRow = 0; startRow < totalRows; startRow += rowsPerPage) {
        if (page > 1) printer.newPage();

        // Заголовок
        QFont titleFont = painter.font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        painter.setFont(titleFont);

        QString title = tr("Отчет по отработанным территориям и странам");
        painter.drawText(pageRect.left() + leftMargin + 100, pageRect.top() + 130, title);
        painter.setFont(font);

        // Рисуем заголовок таблицы
        int x = pageRect.left() + leftMargin;
        int y = pageRect.top() + topMargin;

        // Колонка с номерами строк
        painter.drawRect(x, y, rowNumberWidth, rowHeight);
        painter.drawText(x + 5, y + rowHeight - 10, "#");
        x += rowNumberWidth;

        // Остальные заголовки
        for (int col = 0; col < totalCols; ++col) {
            int colWidth;

            if (col == flagColIndex) {
                colWidth = flagColWidth;
            } else if (col == totalCols - 1 && variableCols > 1) {
                // Последний столбец — остаток ширины
                colWidth = usableWidth - rowNumberWidth - flagColWidth - baseColWidth * (variableCols - 1);
            } else {
                colWidth = baseColWidth;
            }
            QRect cellRect(x, y, colWidth, rowHeight);
            painter.drawRect(cellRect);
            QString header = model->headerData(col, Qt::Horizontal).toString();
            painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter | Qt::AlignLeft, header);
            x += colWidth;
        }

        // Печать строк
        y += rowHeight;
        int rowsOnPage = std::min(rowsPerPage, totalRows - startRow);

        for (int i = 0; i < rowsOnPage; ++i) {
            x = pageRect.left() + leftMargin;
            int row = startRow + i;

            // Номер строки
            QRect rowNumRect(x, y, rowNumberWidth, rowHeight);
            painter.drawRect(rowNumRect);
            painter.drawText(rowNumRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter, QString::number(row + 1));
            x += rowNumberWidth;

            // Данные колонок
            for (int col = 0; col < totalCols; ++col) {
                int colWidth;

                if (col == flagColIndex) {
                    colWidth = flagColWidth;
                } else if (col == totalCols - 1 && variableCols > 1) {
                    colWidth = usableWidth - rowNumberWidth - flagColWidth - baseColWidth * (variableCols - 1);
                } else {
                    colWidth = baseColWidth;
                }

                QRect cellRect(x, y, colWidth, rowHeight);
                painter.drawRect(cellRect);

                QModelIndex cellIndex = model->index(row, col);
                QVariant iconData = model->data(cellIndex, Qt::DecorationRole);

                if (iconData.canConvert<QIcon>()) {
                    QIcon icon = qvariant_cast<QIcon>(iconData);
                    QPixmap originalPixmap = icon.pixmap(256, 256);
                    int flagHeight = cellRect.height() - 10;
                    int flagWidth = qMin(cellRect.width() - 10, flagHeight * 4 / 3);
                    QSize scaledSize(flagWidth, flagHeight);

                    QPixmap scaledPixmap = originalPixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QRect targetRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
                                                           scaledPixmap.size(), cellRect.adjusted(5, 5, -5, -5));
                    painter.drawPixmap(targetRect.topLeft(), scaledPixmap);
                } else {
                    QString text = model->data(cellIndex, Qt::DisplayRole).toString();
                    painter.drawText(cellRect.adjusted(5, 0, -5, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
                }
                x += colWidth;
            }
            y += rowHeight;
        }

        // Подвал
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
