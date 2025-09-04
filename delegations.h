#ifndef DELEGATIONS_H
#define DELEGATIONS_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QDate>
#include <QTime>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QFile>
#include <QDebug>





class FormatCallsign : public QStyledItemDelegate {
public:
    FormatCallsign(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        option->font.setBold(true);
        option->displayAlignment = Qt::AlignVCenter | Qt::AlignCenter;
    }
};

class FormatDate : public QStyledItemDelegate {
public:
    FormatDate(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    QString displayText(const QVariant& value, const QLocale&) const {
      QDate date = QDate::fromString(value.toString(), "yyyyMMdd");
      return date.toString("yyyy-MM-dd");
    }
};

class FormatTime : public QStyledItemDelegate {
public:
    FormatTime(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    QString displayText(const QVariant& value, const QLocale&) const {
      QTime time = QTime::fromString(value.toString(), (value.toString().length() == 6) ? "hhmmss" : "hhmm");
      return time.toString("hh:mm:ss");
    }
};

class FormatFreq : public QStyledItemDelegate {
public:
    FormatFreq(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    QString displayText(const QVariant& value, const QLocale&) const {
      return QString::number((double) value.toLongLong() / 1000000, 'f', 6);
    }
};

class FormatSyncState : public QStyledItemDelegate {
public:
    FormatSyncState(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
        QStyledItemDelegate::initStyleOption(option, index);
        //option->displayAlignment = Qt::AlignVCenter | Qt::AlignCenter;
    }

    QString displayText(const QVariant& value, const QLocale&) const {
      //return QString(value.toInt() == 0 ? "Нет" : "Да");
        switch (value.toInt()) {
            //case 0: return "Нет"; break;
            //case 1: return "Да"; break;
            //case 3: return "Ошибка"; break;
            default: return "";
        }
    }
};

class ColorSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    int services = 0;
    mutable QHash<QString, QIcon> flagCache;

    ColorSqlTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase())
        : QSqlTableModel(parent,db) {;}
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
    {
        if(role==Qt::BackgroundRole)
        {
            if(QSqlTableModel::data(this->index(index.row(), 25)).toInt() == 0)
            {
                return QColor(239,81,81);
            }

            if((services == 2)&&((QSqlTableModel::data(this->index(index.row(), 25)).toInt() == 1)||(QSqlTableModel::data(this->index(index.row(), 25)).toInt() == 2)))
            {
                return QColor(239,153,81);
            }

            if((services == 1)&&((QSqlTableModel::data(this->index(index.row(), 25)).toInt() == 1)||(QSqlTableModel::data(this->index(index.row(), 25)).toInt() == 2)))
            {
                return QSqlTableModel::data(index);
            }

            else if(role == Qt::DisplayRole)
            {
                return QSqlTableModel::data(index);
            }
        }

        if(role==Qt::DecorationRole && index.column() == 21)
        {
            if (QSqlQueryModel::data(index, Qt::DisplayRole).toInt() == 1){
                return QIcon(":resources/images/yes.png");
            } else {
                return QIcon(":resources/images/no.png");
            }
        }

        if (role == Qt::DecorationRole && index.column() == 28)
        {
            QString countryCode = QSqlTableModel::data(index, Qt::DisplayRole).toString().toUpper();
            if (countryCode.isEmpty())
                return QVariant();

            // Проверим наличие в кэше
            if (flagCache.contains(countryCode))
                return flagCache[countryCode];

            // Загружаем флаг, если существует
            QString iconPath = QString(":resources/flags/%1.png").arg(countryCode);
            if (QFile::exists(iconPath)) {
                QPixmap pix(iconPath);
                QPixmap scaled = pix.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                flagCache.insert(countryCode, scaled);
                return scaled;
            } else {
                // Можно кешировать и пустой результат, чтобы не проверять снова
                flagCache.insert(countryCode, QPixmap());
                return QVariant();
            }
        }
        //Подавляем текст (DisplayRole) — возвращаем пустую строку
//        if (role == Qt::DisplayRole && index.column() == 28)
//        {
//           return QString();
//        }
       return QSqlTableModel::data(index,role);
    }
 };

class MySortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit MySortFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent),
        primaryColumn(0), secondaryColumn(1),
        ascendingPrimary(true), ascendingSecondary(true) {}

    void setPrimaryColumn(int col) {
        primaryColumn = col;
        invalidate();
    }

    void setSecondaryColumn(int col) {
        secondaryColumn = col;
        invalidate();
    }

    void setPrimaryOrder(bool ascending) {
        ascendingPrimary = ascending;
        invalidate();
    }

    void setSecondaryOrder(bool ascending) {
        ascendingSecondary = ascending;
        invalidate();
    }

protected:
    int primaryColumn;
    int secondaryColumn;
    bool ascendingPrimary;
    bool ascendingSecondary;

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        QVariant leftFirst = sourceModel()->data(sourceModel()->index(left.row(), primaryColumn));
        QVariant rightFirst = sourceModel()->data(sourceModel()->index(right.row(), primaryColumn));

        if (leftFirst != rightFirst) {
            return (leftFirst < rightFirst) == ascendingPrimary;
        } else {
            QVariant leftSecond = sourceModel()->data(sourceModel()->index(left.row(), secondaryColumn));
            QVariant rightSecond = sourceModel()->data(sourceModel()->index(right.row(), secondaryColumn));
            return (leftSecond < rightSecond) == ascendingSecondary;
        }
    }
};

#endif // DELEGATIONS_H
