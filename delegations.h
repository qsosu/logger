#ifndef DELEGATIONS_H
#define DELEGATIONS_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QDate>
#include <QTime>
#include <QSqlTableModel>
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

    ColorSqlTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase())
        : QSqlTableModel(parent,db) {;}
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
    {
        if(role==Qt::BackgroundRole)
        {
            if(QSqlTableModel::data(this->index(index.row(), 26)).toInt() == 0)
            {
                return QColor(239,81,81);
            }

            if((services == 2)&&((QSqlTableModel::data(this->index(index.row(), 26)).toInt() == 1)||(QSqlTableModel::data(this->index(index.row(), 26)).toInt() == 2)))
            {
                return QColor(239,153,81);
            }

            if((services == 1)&&((QSqlTableModel::data(this->index(index.row(), 26)).toInt() == 1)||(QSqlTableModel::data(this->index(index.row(), 26)).toInt() == 2)))
            {
                return QSqlTableModel::data(index);
            }

            else if(role == Qt::DisplayRole)
            {
                return QSqlTableModel::data(index);
            }
        }

        if(role==Qt::DecorationRole && index.column() == 22)
        {
            if (QSqlQueryModel::data(index, Qt::DisplayRole).toInt() == 1){
                return QIcon(":resources/images/yes.png");
            } else {
                return QIcon(":resources/images/no.png");
            }
        }
        return QSqlTableModel::data(index,role);
    }
};

#endif // DELEGATIONS_H
