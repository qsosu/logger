#include "qsoedit.h"
#include "ui_qsoedit.h"
#include <QDebug>
#include <QSqlError>
#include <QPixmap>
#include <QList>




Qsoedit::Qsoedit(QSqlDatabase db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Qsoedit)
{
    this->db = db;
    ui->setupUi(this);
    setWindowTitle("Редактирование QSO");
    load_flag = false;

    settings = new Settings();
    qrz = new QrzruCallbook(settings->QrzruLogin, settings->QrzruPassword);
    connect(qrz, &QrzruCallbook::error404, this, [=]() {
        QMessageBox::information(this, "Ответ QRZ.RU", tr("Позывной не найден!"));
    });
    connect(qrz, &QrzruCallbook::error, this, [=]() {
        QMessageBox::information(this, "Ответ QRZ.RU", tr("QRZ API - ошибка запроса!"));
    });
    connect(qrz, SIGNAL(loaded(QPixmap)), this, SLOT(loadImage(QPixmap)));

    api = new HttpApi(db, settings->accessToken);
    api->getListBand(); //Загрузка диапазонов
    api->getListSubmodeDropDown(); //Загрузка списков модуляции

    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true); // Выполнить только один раз
    connect(resizeTimer, &QTimer::timeout, this, &Qsoedit::onResizeFinished);
}

Qsoedit::~Qsoedit()
{
    delete ui;
    delete settings;
    delete qrz;
    delete resizeTimer;
}

void Qsoedit::ShowQSOParams(QVariantList data)
{
    dbid = data.at(0).toInt();
    ui->CalsignlineEdit->setText(data.at(1).toString());
    QDate qsoDate = QDate::fromString(data.at(2).toString(), "yyyyMMdd");
    ui->dateEdit->setDate(qsoDate);
    QTime start_time = QTime::fromString(data.at(3).toString(), "hhmmss");
    ui->qso_timeStartEdit->setTime(start_time);
    QTime end_time = QTime::fromString(data.at(4).toString(), "hhmmss");
    ui->QSO_timeEndEdit->setTime(end_time);
    ui->freq_lineEdit->setText(data.at(7).toString());
    ui->name_lineEdit->setText(data.at(8).toString());
    ui->qth_lineEdit->setText(data.at(9).toString());
    ui->rstr_lineEdit->setText(data.at(10).toString());
    ui->rsts_lineEdit->setText(data.at(11).toString());
    ui->qthloc_lineEdit->setText(data.at(12).toString());
    ui->rda_lineEdit->setText(data.at(13).toString());
    ui->comment_lineEdit->setText(data.at(14).toString());

    ui->mode_comboBox->clear();
    if(api->modulations.count() > 0) ui->mode_comboBox->addItems(api->modulations);
    ui->band_comboBox->clear();
    if(api->bands.count() > 0) ui->band_comboBox->addItems(api->bands);
    ui->band_comboBox->setCurrentText(data.at(5).toString());
    ui->mode_comboBox->setCurrentText(data.at(6).toString());
}

void Qsoedit::on_QRZUpdateButton_clicked()
{
    QStringList data = qrz->Get(ui->CalsignlineEdit->text());
    ui->name_lineEdit->setText((data.at(0).length() > 0) ? data.at(0) : "");
    ui->qth_lineEdit->setText((data.at(1).length() > 0) ? data.at(1) : "");
    ui->qthloc_lineEdit->setText((data.at(2).length() > 0) ? data.at(2).toUpper() : "");
    ui->rda_lineEdit->setText((data.at(3).length() > 0) ? data.at(3).toUpper() : "");
    image = ((data.at(4).length() > 0) ? data.at(4) : "");
    if(image != "") {
        qrz->LoadPhoto(image);
        load_flag = true;
    }
}

void Qsoedit::loadImage(QPixmap pix)
{
    pixmap_item = new QGraphicsPixmapItem();
    scene = new QGraphicsScene;

    ui->graphicsView->setScene(scene);
    scene->addItem(pixmap_item);
    pixmap_item->setVisible(true);
    pixmap_item->setPixmap(pix);
    scene->setSceneRect(0, 0, pix.width(), pix.height());
    ui->graphicsView->fitInView(pixmap_item, Qt::KeepAspectRatio);
}


void Qsoedit::on_saveButton_clicked()
{
    QSqlQuery query(db);
    query.prepare("UPDATE records SET NAME = :name, QTH = :qth, GRIDSQUARE = :grid, CNTY = :rda WHERE id=:id");
    query.bindValue(":id", dbid);
    query.bindValue(":name",  ui->name_lineEdit->text());
    query.bindValue(":qth",  ui->qth_lineEdit->text());
    query.bindValue(":grid",  ui->qthloc_lineEdit->text());
    query.bindValue(":rda",  ui->rda_lineEdit->text());

    if(!query.exec()){
        qDebug() << "ERROR UPDATE TABLE record " << query.lastError().text();
     } else {
        qDebug() << "Record " << dbid << " updated.";
        emit db_updated();
     }
    close();
}


void Qsoedit::on_cancelButton_clicked()
{
    close();
}

void Qsoedit::closeEvent(QCloseEvent *event)
{
    if(load_flag) {
        scene->removeItem(pixmap_item);
        delete pixmap_item;
        load_flag = false;
    }
    event->accept();
}

void Qsoedit::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    // Перезапускаем таймер при каждом изменении размера
    resizeTimer->start(300); // Установить задержку в 300 мс
}

void Qsoedit::onResizeFinished()
{
      // Действия после окончания изменения размера окна
      qDebug() << "Изменение размера окна завершено.";
      if(image != "") {
          qrz->LoadPhoto(image);
          load_flag = true;
      }
}
