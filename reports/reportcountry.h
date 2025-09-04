#ifndef REPORTCOUNTRY_H
#define REPORTCOUNTRY_H

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


class ColorTableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ColorTableModel(QObject *parent = nullptr)
        : QStandardItemModel(parent) {}

    // Кэш для иконок флагов
    mutable QHash<QString, QIcon> flagCache;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) return QVariant();

        // Обработка декорации (иконки флага) в первом столбце
        if (role == Qt::DecorationRole && index.column() == 0)
        {
            QString countryCode = QStandardItemModel::data(index, Qt::DisplayRole).toString().toUpper();
            if (countryCode.isEmpty())
                return QVariant();

            // Проверка кэша
            if (flagCache.contains(countryCode))
                return flagCache.value(countryCode);

            // Загрузка иконки из ресурсов
            QString iconPath = QString(":resources/flags/%1.png").arg(countryCode);
            if (QFile::exists(iconPath)) {
                QPixmap pix(iconPath);
                QPixmap scaled = pix.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QIcon icon(scaled);
                flagCache.insert(countryCode, icon);
                return icon;
            } else {
                flagCache.insert(countryCode, QIcon());  // пустой флаг
                return QVariant();
            }
        }
        // Остальные случаи — стандартное поведение
        return QStandardItemModel::data(index, role);
    }
};

namespace Ui {
class ReportCountry;
}

class ReportCountry : public QDialog
{
    Q_OBJECT

public:
    explicit ReportCountry(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportCountry();
    void printTableAdvanced(QTableView *tableView, QWidget *parent = nullptr);

private slots:
    void on_CloseButton_clicked();
    void on_PrintButton_clicked();

private:
    Ui::ReportCountry *ui;
    QSqlDatabase db;
};

#endif // REPORTCOUNTRY_H
