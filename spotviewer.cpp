/**********************************************************************************************************
Description :  Implementation of the SpotViewer class, which provides an interactive window for browsing,
            :  filtering, and searching radio spots stored in the local database. The viewer allows
            :  operators to quickly find active DX stations, control the CAT interface by double-clicking
            :  on a spot, and manage the table view layout persistently.
Version     :  1.4.0
Date        :  20.08.2025
Author      :  R9JAU
Comments    :  - Provides filter panel (mode, band, continent, country) with smooth animation toggle.
            :  - Supports direct CAT control: double-click on a spot sets rig frequency and mode.
            :  - Implements search dialog with callsign validation and case normalization.
            :  - Adaptive UI with animated filter visibility.
**********************************************************************************************************/

#include "spotviewer.h"
#include "ui_spotviewer.h"

#include <QSortFilterProxyModel>
#include <QList>
#include <QSqlError>
#include <QDebug>
#include <QTimer>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>



SpotViewer::SpotViewer(QSqlDatabase db, QList<bandData> bList, QList<modeData> mList, QVector<CountryEntry>& entries, cat_Interface *cat, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpotViewer)
{
    this->db = db;
    this->entries = entries;
    this->CAT = cat;
    ui->setupUi(this);
    this->bandList.append(bList);
    this->modeList.append(mList);

    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    flags |= Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;
    this->setWindowFlags(flags);

    // подключаем кнопку фильтров
    connect(ui->filterButton, &QPushButton::clicked, this, &SpotViewer::toggleFilters);

    //Заполняем Combobox
    ui->bandCombo->clear();
    ui->bandCombo->addItem(tr("Все"));
    for(int j = 0; j < bandList.count(); j++)
    {
        ui->bandCombo->addItem(bandList.at(j).band_name);
    }

    ui->modeCombo->clear();
    ui->modeCombo->addItem(tr("Все"));
    for(int j = 0; j < modeList.count(); j++)
    {
        ui->modeCombo->addItem(modeList.at(j).mode_value);
    }

    QStringList countries;
    for (int j = 0; j < entries.count(); ++j) {
        QString country = entries.at(j).country;

        // Если страна содержит "Russia", то заменяем её на "Russia"
        if (country.endsWith("Russia")) {
             country = "Russia";
        }
        if (!countries.contains(country))
            countries << country;
    }

    // сортируем по алфавиту
    countries.sort(Qt::CaseInsensitive);

    // очищаем и добавляем в combobox
    ui->countryCombo->clear();
    ui->countryCombo->addItem(tr("Все"));
    ui->countryCombo->addItems(countries);

    // сначала скрываем блок фильтров
    ui->filterGroupBox->setMaximumHeight(0);
    ui->filterGroupBox->setVisible(false);

    // подключаем фильтры к обновлению данных
    connect(ui->modeCombo, &QComboBox::currentTextChanged, this, &SpotViewer::loadData);
    connect(ui->bandCombo, &QComboBox::currentTextChanged, this, &SpotViewer::loadData);
    connect(ui->contCombo, &QComboBox::currentTextChanged, this, &SpotViewer::loadData);
    connect(ui->countryCombo, &QComboBox::currentTextChanged, this, &SpotViewer::loadData);

    model = new ColorSQLTableModel(this);
    ui->tableView->setModel(model);

    // Разрешаем перетаскивание столбцов
    ui->tableView->horizontalHeader()->setSectionsMovable(true);
    ui->tableView->horizontalHeader()->setDragEnabled(true);
    ui->tableView->horizontalHeader()->setDragDropMode(QAbstractItemView::InternalMove);

    // Восстанавливаем порядок столбцов
    restoreHeaderState(ui->tableView);
    QTimer::singleShot(0, this, &SpotViewer::loadData);


//    for (int j = 0; j < entries.count(); ++j)
//    {
//        QString country = entries.at(j).country;
//        QString country_code = countryToIso.value(country, "");

//        QString iconPath = QString(":resources/flags/%1.png").arg(country_code);

//        if (QFile::exists(iconPath))
//        {
//            qDebug() << entries.at(j).country << " " << country_code << " " << iconPath;
//        } else {
//            qDebug() << entries.at(j).country << " " << country_code << " " << "None";
//        }
//    }
//    // сортируем по алфавиту
//    countries.sort(Qt::CaseInsensitive);
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::toggleFilters()
{
    bool opening = !ui->filterGroupBox->isVisible();
    int startHeight = opening ? 0 : ui->filterGroupBox->height();
    int endHeight   = opening ? ui->filterGroupBox->sizeHint().height() : 0;

    QPropertyAnimation *anim = new QPropertyAnimation(ui->filterGroupBox, "maximumHeight");
    anim->setDuration(300);
    anim->setStartValue(startHeight);
    anim->setEndValue(endHeight);
    anim->setEasingCurve(QEasingCurve::InOutCubic);

    ui->filterGroupBox->setVisible(true); // показываем перед анимацией
    connect(anim, &QPropertyAnimation::finished, this, [this, opening]() {
        if (!opening) ui->filterGroupBox->setVisible(false);
        ui->filterButton->setText(opening ? tr("Скрыть фильтры ▶") : tr("Показать фильтры ▼"));
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::loadData()
{
    QString sql = "SELECT SPOTTER, CALLSIGN, DXCC_COUNTRY_CODE, DXCC_COUNTRY, DXCC_CONTINENT, MODE, BAND, FREQUENCY, MESSAGE, EVENT_AT FROM SPOTS";
    QStringList conditions;

    QString mode = ui->modeCombo->currentText();
    if (!mode.isEmpty() && mode != tr("Все"))
        conditions << QString("MODE = '%1'").arg(mode);

    QString band = ui->bandCombo->currentText();
    if (!band.isEmpty() && band != tr("Все"))
        conditions << QString("BAND = '%1'").arg(band);

    QString cont = ui->contCombo->currentText();
    if (!cont.isEmpty() && cont != tr("Все")) {
        QString code = continentMap.value(cont, "");
        if (!code.isEmpty()) {
            conditions << QString("DXCC_CONTINENT = '%1'").arg(code);
        }
    }

    QString country = ui->countryCombo->currentText();
    if (!country.isEmpty() && country != tr("Все")) {
        if (country == "Russia") {
            conditions << "DXCC_COUNTRY LIKE 'Russia%'";
        } else {
            conditions << QString("DXCC_COUNTRY = '%1'").arg(country.replace("'", "''"));
        }
    }

    if (!conditions.isEmpty())
        sql += " WHERE " + conditions.join(" AND ");

    sql += " ORDER BY ID DESC";

    model->setQuery(sql, db);

    if (model->lastError().isValid()) qWarning() << "SQL error:" << model->lastError().text();

        model->setHeaderData(0, Qt::Horizontal, tr("Спотер"));
        model->setHeaderData(1, Qt::Horizontal, tr("Позывной DX"));
        model->setHeaderData(2, Qt::Horizontal, tr("Флаг"));
        model->setHeaderData(3, Qt::Horizontal, tr("Страна"));
        model->setHeaderData(4, Qt::Horizontal, tr("Континент"));
        model->setHeaderData(5, Qt::Horizontal, tr("Модуляция"));
        model->setHeaderData(6, Qt::Horizontal, tr("Диапазон"));
        model->setHeaderData(7, Qt::Horizontal, tr("Частота"));
        model->setHeaderData(8, Qt::Horizontal, tr("Информация"));
        model->setHeaderData(9, Qt::Horizontal, tr("Дата"));

        ui->tableView->resizeColumnsToContents();
        ui->tableView->resizeRowsToContents();

        auto *header = ui->tableView->horizontalHeader();
        for (int i = 1; i < model->columnCount(); ++i) {
            if (i == 8)
                header->setSectionResizeMode(i, QHeaderView::Stretch);
            else
                header->setSectionResizeMode(i, QHeaderView::Interactive);
        }
        ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->tableView->verticalHeader()->setDefaultSectionSize(22);
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::updateSpots()
{
    loadData();
}
//-----------------------------------------------------------------------------------------------------

SpotViewer::~SpotViewer()
{
    delete ui;
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void SpotViewer::on_pushButton_clicked()
{
    saveHeaderState(ui->tableView);
    close();
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::on_searchButton_clicked()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(QObject::tr("Поиск DX"));
    dialog.setLabelText(QObject::tr("Введите позывной:"));
    dialog.setTextValue("");
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Находим QLineEdit внутри диалога и вешаем валидатор
    if (QLineEdit *lineEdit = dialog.findChild<QLineEdit *>()) {
        auto *validator = new QRegularExpressionValidator(QRegularExpression("^[A-Za-z0-9/]*$"), lineEdit);
        lineEdit->setValidator(validator);

        QObject::connect(lineEdit, &QLineEdit::textChanged, lineEdit, [lineEdit](const QString &text){
            QString upper = text.toUpper();
            if (text != upper) {
                int pos = lineEdit->cursorPosition();
                lineEdit->setText(upper);
                lineEdit->setCursorPosition(pos);
            }
        });
    }

    // запускаем диалог
    if (dialog.exec() == QDialog::Accepted) {
        QString Call = dialog.textValue();
        QAbstractItemModel *model = ui->tableView->model();

        if (!model) return;
        int rowCount = model->rowCount();
        bool found = false;

        for (int row = 0; row < rowCount; ++row) {
            QModelIndex index = model->index(row, 1); // колонка с позывным
            QString value = model->data(index).toString();
            if (value.compare(Call, Qt::CaseInsensitive) == 0) {
                ui->tableView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                ui->tableView->scrollTo(index, QAbstractItemView::PositionAtCenter);
                found = true;
                break;
            }
        }
        if (!found) {
            QMessageBox::information(this, tr("Поиск DX"), tr("Позывной не найден"));
        }
    }
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::on_tableView_doubleClicked(const QModelIndex &index)
{
   int idx = index.row();

   QString call = model->data(model->index(idx, 1)).toString();
   QString band = model->data(model->index(idx, 6)).toString();
   QString mode = model->data(model->index(idx, 5)).toString();
   QVariant val = model->data(model->index(idx, 7));
   QString strVal = val.toString();

   long long freqHz = 0;

   if (strVal.contains('.')) {
       // Значение в МГц → переводим в Гц
       double freqMhz = strVal.toDouble();
       freqHz = static_cast<long long>(freqMhz * 1000000.0);
   } else {
       // Значение уже в Гц
       freqHz = strVal.toLongLong();
   }

   CAT->setFreq(freqHz);
   CAT->modeNameToMode(mode);
   qDebug() << "Позывной:" << call << "Частота:" << freqHz << "Гц" << "Диапазон:" << band << "Модуляция:" << mode;
   emit setNewQSO(call, band, freqHz, mode);
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::saveHeaderState(QTableView *tableView)
{
    QSettings settings("QSO.SU", "QSOLogger");
    settings.setValue("headerStateSpot", tableView->horizontalHeader()->saveState());
}
//-----------------------------------------------------------------------------------------------------

void SpotViewer::restoreHeaderState(QTableView *tableView)
{
    QSettings settings("QSO.SU", "QSOLogger");
    QByteArray state = settings.value("headerStateSpot").toByteArray();
    if (!state.isEmpty()) {
        tableView->horizontalHeader()->restoreState(state);
    }
}
//-----------------------------------------------------------------------------------------------------
