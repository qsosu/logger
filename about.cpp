#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    ui->Info->setFont(QFont("Roboto", 9, QFont::Normal, false));
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    close();
}

