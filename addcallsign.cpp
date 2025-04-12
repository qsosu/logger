#include "addcallsign.h"
#include "ui_addcallsign.h"
#include <QMessageBox>

Addcallsign::Addcallsign(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Addcallsign)
{
    ui->setupUi(this);
    ui->addCallsignEdit->setStyleSheet("color: black; font-weight: bold");
    ui->addCallsignEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9/]*$"), this));
    ui->addITU->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-9]|[1-8][0-9]|90)$"), this));
    ui->addCQ->setValidator(new QRegularExpressionValidator(QRegularExpression("^(?:[1-9]|[1-3][0-9]|40)$"), this));
}

Addcallsign::~Addcallsign()
{
    delete ui;
}

void Addcallsign::on_addCallsignEdit_textEdited(const QString &arg1)
{
    QString callsign = arg1.toUpper();
    ui->addCallsignEdit->setText(callsign);
    ui->addCallsignEdit->setStyleSheet("font-weight: bold");
}

void Addcallsign::on_addRDA_textEdited(const QString &arg1)
{
    QString rda = arg1.toUpper();
    ui->addRDA->setText(rda);
}

void Addcallsign::on_addLOC_textEdited(const QString &arg1)
{
    QString location = arg1.toUpper();
    ui->addLOC->setText(location);
}

void Addcallsign::on_addCallsignType_currentIndexChanged(int index)
{
    if(index == 0) add_CallsignType = 0;
    if(index == 1) add_CallsignType = 1;
    if(index == 2) add_CallsignType = 2;
}


void Addcallsign::on_OkCallsignBtn_clicked()
{
    QDateTime dt;
    add_Callsign = ui->addCallsignEdit->text();
    add_CallsignType = ui->addCallsignType->currentIndex();
    dt = ui->validity_start->dateTime();
    add_validity_start = dt.toSecsSinceEpoch();
    dt = ui->validity_stop->dateTime();
    add_validity_stop = dt.toSecsSinceEpoch();
    add_location = ui->addLOC->text();
    add_rda = ui->addRDA->text();
    add_ituz = ui->addITU->text().toInt();
    add_cqz = ui->addCQ->text().toInt();

    if (add_Callsign.length() < 3) {
        QMessageBox::critical(0, "Ошибка!", "Позывной сигнал не может быть меньше 3 символов.", QMessageBox::Ok);
        return;
    }
    if (add_location.length() < 4) {
        QMessageBox::critical(0, "Ошибка!", "QTH локатор не может быть меньше 4 символов.", QMessageBox::Ok);
        return;
    }
    emit addCallsign();
    close();
}

void Addcallsign::on_CnlCallsignBtn_clicked()
{
    close();
}

