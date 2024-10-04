/********************************************************************************
** Form generated from reading UI file 'addcallsigns.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDCALLSIGNS_H
#define UI_ADDCALLSIGNS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QLineEdit *addCallsignEdit;
    QLabel *label;
    QLabel *label_2;
    QComboBox *comboBox;
    QDateEdit *dateEdit;
    QDateEdit *dateEdit_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *lineEdit;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLineEdit *lineEdit_4;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName("Dialog");
        Dialog->resize(360, 287);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resources/images/logo_mini.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Dialog->setWindowIcon(icon);
        Dialog->setModal(true);
        addCallsignEdit = new QLineEdit(Dialog);
        addCallsignEdit->setObjectName("addCallsignEdit");
        addCallsignEdit->setGeometry(QRect(20, 30, 111, 22));
        label = new QLabel(Dialog);
        label->setObjectName("label");
        label->setGeometry(QRect(20, 10, 101, 16));
        label_2 = new QLabel(Dialog);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(20, 60, 49, 16));
        comboBox = new QComboBox(Dialog);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName("comboBox");
        comboBox->setGeometry(QRect(20, 80, 111, 22));
        dateEdit = new QDateEdit(Dialog);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setGeometry(QRect(20, 130, 110, 22));
        dateEdit->setCalendarPopup(true);
        dateEdit_2 = new QDateEdit(Dialog);
        dateEdit_2->setObjectName("dateEdit_2");
        dateEdit_2->setGeometry(QRect(20, 180, 110, 22));
        dateEdit_2->setCalendarPopup(true);
        label_3 = new QLabel(Dialog);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(20, 110, 121, 16));
        label_4 = new QLabel(Dialog);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(20, 160, 141, 16));
        label_5 = new QLabel(Dialog);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(210, 10, 71, 16));
        lineEdit = new QLineEdit(Dialog);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(210, 30, 113, 22));
        label_6 = new QLabel(Dialog);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(210, 70, 49, 16));
        label_7 = new QLabel(Dialog);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(210, 120, 49, 16));
        label_8 = new QLabel(Dialog);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(210, 170, 49, 16));
        lineEdit_2 = new QLineEdit(Dialog);
        lineEdit_2->setObjectName("lineEdit_2");
        lineEdit_2->setGeometry(QRect(210, 90, 113, 22));
        lineEdit_3 = new QLineEdit(Dialog);
        lineEdit_3->setObjectName("lineEdit_3");
        lineEdit_3->setGeometry(QRect(210, 140, 113, 22));
        lineEdit_4 = new QLineEdit(Dialog);
        lineEdit_4->setObjectName("lineEdit_4");
        lineEdit_4->setGeometry(QRect(210, 190, 113, 22));
        pushButton = new QPushButton(Dialog);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(50, 240, 75, 24));
        pushButton_2 = new QPushButton(Dialog);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(200, 240, 75, 24));

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QCoreApplication::translate("Dialog", "\320\224\320\276\320\261\320\260\320\262\320\273\320\265\320\275\320\270\320\265 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\263\320\276", nullptr));
        label->setText(QCoreApplication::translate("Dialog", "\320\237\320\276\320\267\321\213\320\262\320\275\320\276\320\271 \321\201\320\270\320\263\320\275\320\260\320\273", nullptr));
        label_2->setText(QCoreApplication::translate("Dialog", "\320\242\320\270\320\277", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("Dialog", "\320\236\321\201\320\275\320\276\320\262\320\275\320\276\320\271", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("Dialog", "\320\241\320\277\320\265\321\206\320\270\320\260\320\273\321\214\320\275\321\213\320\271", nullptr));

        label_3->setText(QCoreApplication::translate("Dialog", "\320\224\320\260\321\202\320\260 \320\275\320\260\321\207\320\260\320\273\320\260 \320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217", nullptr));
        label_4->setText(QCoreApplication::translate("Dialog", "\320\224\320\260\321\202\320\260 \320\276\320\272\320\276\320\275\321\207\320\260\320\275\320\270\321\217 \320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217", nullptr));
        label_5->setText(QCoreApplication::translate("Dialog", "RDA  County", nullptr));
        label_6->setText(QCoreApplication::translate("Dialog", "LOC", nullptr));
        label_7->setText(QCoreApplication::translate("Dialog", "ITU", nullptr));
        label_8->setText(QCoreApplication::translate("Dialog", "CQ", nullptr));
        pushButton->setText(QCoreApplication::translate("Dialog", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Dialog", "\320\236\321\202\320\274\320\265\320\275\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDCALLSIGNS_H
