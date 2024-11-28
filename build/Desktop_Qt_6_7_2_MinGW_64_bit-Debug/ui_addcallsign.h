/********************************************************************************
** Form generated from reading UI file 'addcallsign.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDCALLSIGN_H
#define UI_ADDCALLSIGN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_Addcallsign
{
public:
    QGridLayout *gridLayout;
    QPushButton *CnlCallsignBtn;
    QPushButton *OkCallsignBtn;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLineEdit *addLOC;
    QLineEdit *addITU;
    QLabel *label_5;
    QDateEdit *validity_stop;
    QLabel *label_4;
    QLineEdit *addCQ;
    QLineEdit *addRDA;
    QLabel *label_3;
    QLabel *label_6;
    QLineEdit *addCallsignEdit;
    QComboBox *addCallsignType;
    QDateEdit *validity_start;
    QLabel *label_7;
    QLabel *label;
    QLabel *label_8;

    void setupUi(QDialog *Addcallsign)
    {
        if (Addcallsign->objectName().isEmpty())
            Addcallsign->setObjectName("Addcallsign");
        Addcallsign->resize(500, 280);
        Addcallsign->setMinimumSize(QSize(500, 280));
        Addcallsign->setMaximumSize(QSize(500, 280));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resources/images/icon32.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Addcallsign->setWindowIcon(icon);
        gridLayout = new QGridLayout(Addcallsign);
        gridLayout->setObjectName("gridLayout");
        CnlCallsignBtn = new QPushButton(Addcallsign);
        CnlCallsignBtn->setObjectName("CnlCallsignBtn");

        gridLayout->addWidget(CnlCallsignBtn, 12, 2, 1, 1);

        OkCallsignBtn = new QPushButton(Addcallsign);
        OkCallsignBtn->setObjectName("OkCallsignBtn");

        gridLayout->addWidget(OkCallsignBtn, 12, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 12, 0, 1, 1);

        groupBox = new QGroupBox(Addcallsign);
        groupBox->setObjectName("groupBox");
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName("gridLayout_2");
        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        gridLayout_2->addWidget(label_2, 0, 1, 1, 1);

        addLOC = new QLineEdit(groupBox);
        addLOC->setObjectName("addLOC");

        gridLayout_2->addWidget(addLOC, 6, 1, 1, 1);

        addITU = new QLineEdit(groupBox);
        addITU->setObjectName("addITU");

        gridLayout_2->addWidget(addITU, 13, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        gridLayout_2->addWidget(label_5, 5, 0, 1, 1);

        validity_stop = new QDateEdit(groupBox);
        validity_stop->setObjectName("validity_stop");
        validity_stop->setDateTime(QDateTime(QDate(2023, 12, 31), QTime(4, 0, 0)));
        validity_stop->setCalendarPopup(true);

        gridLayout_2->addWidget(validity_stop, 4, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        gridLayout_2->addWidget(label_4, 3, 1, 1, 1);

        addCQ = new QLineEdit(groupBox);
        addCQ->setObjectName("addCQ");

        gridLayout_2->addWidget(addCQ, 13, 1, 1, 1);

        addRDA = new QLineEdit(groupBox);
        addRDA->setObjectName("addRDA");

        gridLayout_2->addWidget(addRDA, 6, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        gridLayout_2->addWidget(label_3, 3, 0, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        gridLayout_2->addWidget(label_6, 5, 1, 1, 1);

        addCallsignEdit = new QLineEdit(groupBox);
        addCallsignEdit->setObjectName("addCallsignEdit");

        gridLayout_2->addWidget(addCallsignEdit, 2, 0, 1, 1);

        addCallsignType = new QComboBox(groupBox);
        addCallsignType->addItem(QString());
        addCallsignType->addItem(QString());
        addCallsignType->addItem(QString());
        addCallsignType->setObjectName("addCallsignType");
        addCallsignType->setToolTipDuration(-1);

        gridLayout_2->addWidget(addCallsignType, 2, 1, 1, 1);

        validity_start = new QDateEdit(groupBox);
        validity_start->setObjectName("validity_start");
        validity_start->setDateTime(QDateTime(QDate(2023, 12, 31), QTime(4, 0, 0)));
        validity_start->setCalendarPopup(true);

        gridLayout_2->addWidget(validity_start, 4, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        gridLayout_2->addWidget(label_7, 12, 0, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName("label");

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        gridLayout_2->addWidget(label_8, 12, 1, 1, 1);


        gridLayout->addWidget(groupBox, 1, 0, 1, 3);


        retranslateUi(Addcallsign);

        QMetaObject::connectSlotsByName(Addcallsign);
    } // setupUi

    void retranslateUi(QDialog *Addcallsign)
    {
        Addcallsign->setWindowTitle(QCoreApplication::translate("Addcallsign", "\320\224\320\276\320\261\320\260\320\262\320\273\320\265\320\275\320\270\320\265 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\263\320\276", nullptr));
        CnlCallsignBtn->setText(QCoreApplication::translate("Addcallsign", "\320\236\321\202\320\274\320\265\320\275\320\260", nullptr));
        OkCallsignBtn->setText(QCoreApplication::translate("Addcallsign", "\320\236\320\232", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Addcallsign", "\320\235\320\276\320\262\321\213\320\271 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\271 ", nullptr));
        label_2->setText(QCoreApplication::translate("Addcallsign", "\320\242\320\270\320\277", nullptr));
#if QT_CONFIG(tooltip)
        addLOC->setToolTip(QCoreApplication::translate("Addcallsign", "<html><head/><body><p>QTH-\320\273\320\276\320\272\320\260\321\202\320\276\321\200 - \321\201\320\270\321\201\321\202\320\265\320\274\320\260 \320\277\321\200\320\270\320\261\320\273\320\270\320\266\321\221\320\275\320\275\320\276\320\263\320\276 \321\203\320\272\320\260\320\267\320\260\320\275\320\270\321\217 \320\274\320\265\321\201\321\202\320\276\320\277\320\276\320\273\320\276\320\266\320\265\320\275\320\270\321\217 \320\276\320\261\321\212\320\265\320\272\321\202\320\260 \320\275\320\260 \320\277\320\276\320\262\320\265\321\200\321\205\320\275\320\276\321\201\321\202\320\270 \320\227\320\265\320\274\320\273\320\270, \320\277\321\200\320\270\320\275\321\217\321\202\320\260\321\217 \320\262 \320\273\321\216\320\261\320\270\321\202\320\265\320\273\321\214\321\201\320\272\320\276\320\271 \321\200\320\260\320\264\320\270\320\276\321\201\320\262\321\217\320\267\320\270.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_5->setText(QCoreApplication::translate("Addcallsign", "RDA  County", nullptr));
#if QT_CONFIG(tooltip)
        validity_stop->setToolTip(QCoreApplication::translate("Addcallsign", "\320\224\320\260\321\202\320\260 \320\277\321\200\320\265\320\272\321\200\320\260\321\211\320\265\320\275\320\270\320\265 \320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\263\320\276", nullptr));
#endif // QT_CONFIG(tooltip)
        label_4->setText(QCoreApplication::translate("Addcallsign", "\320\224\320\260\321\202\320\260 \320\276\320\272\320\276\320\275\321\207\320\260\320\275\320\270\321\217 \320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217", nullptr));
#if QT_CONFIG(tooltip)
        addRDA->setToolTip(QCoreApplication::translate("Addcallsign", "<html><head/><body><p align=\"justify\">\320\243\320\275\320\270\320\272\320\260\320\273\321\214\320\275\321\213\320\271 \320\275\320\276\320\274\320\265\321\200 RDA, \321\201\320\276\321\201\321\202\320\276\321\217\321\211\320\270\320\271 \320\270\320\267 \321\201\320\276\320\272\321\200\320\260\321\211\320\265\320\275\320\275\320\276\320\263\320\276 \320\275\320\260\320\267\320\262\320\260\320\275\320\270\321\217 \320\236\320\261\320\273\320\260\321\201\321\202\320\270, \320\277\321\200\320\270\320\274\320\265\320\275\321\217\320\265\320\274\320\276\320\263\320\276 \320\262 Russian DX Contest rdxc.org \320\270 \320\264\320\262\321\203\321\205\320\267\320\275\320\260\321\207\320\275\320\276\320\263\320\276 \321\207\320\270\321\201\320\273\320\260</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_3->setText(QCoreApplication::translate("Addcallsign", "\320\224\320\260\321\202\320\260 \320\275\320\260\321\207\320\260\320\273\320\260 \320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217", nullptr));
        label_6->setText(QCoreApplication::translate("Addcallsign", "LOC", nullptr));
#if QT_CONFIG(tooltip)
        addCallsignEdit->setToolTip(QCoreApplication::translate("Addcallsign", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\320\235\320\265\320\276\320\261\321\205\320\276\320\264\320\270\320\274\320\276 \320\267\320\260\320\277\320\276\320\273\320\275\320\270\321\202\321\214 \302\253\320\237\320\276\320\267\321\213\320\262\320\275\320\276\320\271\302\273</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        addCallsignType->setItemText(0, QCoreApplication::translate("Addcallsign", "\320\236\321\201\320\275\320\276\320\262\320\275\320\276\320\271", nullptr));
        addCallsignType->setItemText(1, QCoreApplication::translate("Addcallsign", "\320\224\320\276\320\277\320\276\320\273\320\275\320\270\321\202\320\265\320\273\321\214\320\275\321\213\320\271", nullptr));
        addCallsignType->setItemText(2, QCoreApplication::translate("Addcallsign", "\320\241\320\277\320\265\321\206\320\270\320\260\320\273\321\214\320\275\321\213\320\271", nullptr));

#if QT_CONFIG(tooltip)
        addCallsignType->setToolTip(QCoreApplication::translate("Addcallsign", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\320\236\321\201\320\275\320\276\320\262\320\275\320\276\320\271, \320\272\320\276\320\273\320\273\320\265\320\272\321\202\320\270\320\262\320\275\321\213\320\271 \320\270\320\273\320\270 \321\201\320\277\320\265\321\206\320\270\320\260\320\273\321\214\320\275\321\213\320\271: \320\232 \320\276\321\201\320\275\320\276\320\262\320\275\321\213\320\274 \320\270\320\273\320\270 \320\272 \321\201\320\277"
                        "\320\265\321\206\320\270\320\260\320\273\321\214\320\275\321\213\320\274 \320\277\320\276\320\267\321\213\320\262\320\275\321\213\320\274, \320\276\321\202\320\275\320\276\321\201\321\217\321\202\321\201\321\217 \320\277\320\276\320\267\321\213\320\262\320\275\321\213\320\265 \320\272\320\276\321\202\320\276\321\200\321\213\320\265 \320\261\321\213\320\273\320\270 \320\262\321\213\320\264\320\260\320\275\321\213 \320\263\320\276\321\201\321\203\320\264\320\260\321\200\321\201\321\202\320\262\320\265\320\275\320\275\321\213\320\274 \320\276\321\200\320\263\320\260\320\275\320\276\320\274. <br />\320\237\321\200\320\270\320\274\320\265\321\200: R1XXX \320\270\320\273\320\270 R2024K. </p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">\320\224\320\276\320\277\320\276\320\273\320\275\320\270\321\202\320\265\320\273\321\214\320\275\321\213\320\271: \320\232 \320\264\320\276\320\277\320\276\320\273\320\275\320\270\321\202\320\265\320\273"
                        "\321\214\320\275\321\213\320\274 \321\201\320\270\320\263\320\275\320\260\320\273\320\260\320\274 \320\276\321\202\320\275\320\276\321\201\321\217\321\202\321\201\321\217 \320\276\321\201\320\275\320\276\320\262\320\275\321\213\320\265 \320\277\320\276\320\267\321\213\320\262\320\275\321\213\320\265 \321\201 \320\264\321\200\320\276\320\261\321\214\321\216, \320\277\320\265\321\200\320\265\320\264 \320\264\320\276\320\261\320\260\320\262\320\273\320\265\320\275\320\270\320\265\320\274, \320\264\320\276\320\261\320\260\320\262\321\214\321\202\320\265 \320\276\321\201\320\275\320\276\320\262\320\275\320\276\320\271 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\271. \320\237\321\200\320\270\320\274\320\265\321\200: R1XXX/P \320\270\320\273\320\270 7Z/R1XXX/QRP.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        validity_start->setToolTip(QCoreApplication::translate("Addcallsign", "\320\224\320\260\321\202\320\260 \320\277\321\200\320\270\321\201\320\262\320\276\320\265\320\275\320\270\321\217 \320\277\320\276\320\267\321\213\320\262\320\275\320\276\320\263\320\276", nullptr));
#endif // QT_CONFIG(tooltip)
        label_7->setText(QCoreApplication::translate("Addcallsign", "ITU", nullptr));
        label->setText(QCoreApplication::translate("Addcallsign", "\320\237\320\276\320\267\321\213\320\262\320\275\320\276\320\271 \321\201\320\270\320\263\320\275\320\260\320\273", nullptr));
        label_8->setText(QCoreApplication::translate("Addcallsign", "CQ", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Addcallsign: public Ui_Addcallsign {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDCALLSIGN_H
