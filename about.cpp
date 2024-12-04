#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    qApp->setFont(QFont("Roboto", 8, QFont::Normal, false));
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    close();
}

