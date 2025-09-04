#ifndef SPOTVIEWER_H
#define SPOTVIEWER_H

#include <QDialog>
#include <QHeaderView>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QFile>
#include <QTableView>

#include "ham_definitions.h"
#include "updatelogprefix.h"
#include "cat_interface.h"



class ColorSQLTableModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit ColorSQLTableModel(QObject *parent = nullptr)
        : QSqlQueryModel(parent) {}

    // Кэш для иконок флагов
    mutable QHash<QString, QIcon> flagCache;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid()) return QVariant();

        // Обработка декорации (иконки флага) в первом столбце
        if (role == Qt::DecorationRole && index.column() == 2)
        {
            QString countryCode = QSqlQueryModel::data(index, Qt::DisplayRole).toString().toUpper();
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
        return QSqlQueryModel::data(index, role);
    }
};


namespace Ui {
class SpotViewer;
}

class SpotViewer : public QDialog
{
    Q_OBJECT

public:
    explicit SpotViewer(QSqlDatabase db, QList<bandData> bList, QList<modeData> mList, QList<PrefixEntry>& entries, cat_Interface *cat, QWidget *parent = nullptr);
    ~SpotViewer();
    void loadData();
    void toggleFilters();

signals:
    void setNewQSO(QString call, QString band, double freq, QString mode);

private slots:
    void on_pushButton_clicked();
    void on_searchButton_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);

public slots:
    void updateSpots();

private:
    Ui::SpotViewer *ui;
    QSqlDatabase db;
    ColorSQLTableModel *model;
    QList<bandData> bandList;
    QList<modeData> modeList;
    QList<PrefixEntry> entries;

    // Словарь: отображаемое имя → код континента
    QMap<QString, QString> continentMap = {
        {"Африка", "AF"},
        {"Антарктида", "AN"},
        {"Азия", "AS"},
        {"Европа", "EU"},
        {"Северная Америка", "NA"},
        {"Океания", "OC"},
        {"Южная Америка", "SA"}
    };
    cat_Interface *CAT;

    void saveHeaderState(QTableView *tableView);
    void restoreHeaderState(QTableView *tableView);
};


#endif // SPOTVIEWER_H
