/********************************************************************************
** Form generated from reading UI file 'about.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_About
{
public:
    QLabel *label;
    QLabel *label_2;
    QPushButton *pushButton;

    void setupUi(QDialog *About)
    {
        if (About->objectName().isEmpty())
            About->setObjectName("About");
        About->setWindowModality(Qt::WindowModality::WindowModal);
        About->resize(420, 210);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(About->sizePolicy().hasHeightForWidth());
        About->setSizePolicy(sizePolicy);
        About->setMinimumSize(QSize(420, 210));
        About->setBaseSize(QSize(420, 210));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resources/images/icon32.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        About->setWindowIcon(icon);
        label = new QLabel(About);
        label->setObjectName("label");
        label->setGeometry(QRect(10, 20, 71, 31));
        label->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/logo_mini.svg")));
        label->setScaledContents(true);
        label_2 = new QLabel(About);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(100, 10, 311, 151));
        pushButton = new QPushButton(About);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(330, 170, 75, 24));

        retranslateUi(About);

        QMetaObject::connectSlotsByName(About);
    } // setupUi

    void retranslateUi(QDialog *About)
    {
        About->setWindowTitle(QCoreApplication::translate("About", "\320\236 \320\277\321\200\320\276\320\263\321\200\320\260\320\274\320\274\320\265...", nullptr));
        label->setText(QString());
        label_2->setText(QCoreApplication::translate("About", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">QSO Logger \342\200\224 \321\215\321\202\320\276 \320\277\321\200\320\276\321\202\320\276\321\202\320\270\320\277 \320\260\320\277\320\277\320\260\321\200\320\260\321\202\320\275\320\276\320\263\320\276 \320\266\321\203\321\200\320\275\320\260\320\273\320\260 </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-inde"
                        "nt:0; text-indent:0px;\"><span style=\" color:#000000;\">\321\201\320\277\320\276\321\201\320\276\320\261\320\275\320\276\320\263\320\276 \320\277\320\265\321\200\320\265\320\264\320\260\320\262\320\260\321\202\321\214 \320\264\320\260\320\275\320\275\321\213\320\265 \320\275\320\260 QSO.SU \320\270 \320\262\321\213\321\201\321\202\321\203\320\277\320\260\321\202\321\214 </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\320\262 \321\200\320\276\320\273\320\270 \320\260\320\263\320\265\320\275\321\202\320\260 \320\264\320\273\321\217 \320\262\320\267\320\260\320\270\320\274\320\276\320\264\320\265\320\271\321\201\321\202\320\262\320\270\321\217 \321\201 \320\237\320\236 \320\264\320\273\321\217 \321\206\320\270\321\204\321\200\320\276\320\262\321\213\321\205 </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                        "<span style=\" color:#000000;\">\320\262\320\270\320\264\320\276\320\262 \321\201\320\262\321\217\320\267\320\270.</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#000000;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\320\222\320\265\321\200\321\201\320\270\321\217 \320\237\320\236: 1.2.362, \320\262\320\265\321\200\321\201\320\270\321\217 API: 1.0</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\320\220\320\262\321\202\320\276\321\200\321\213: Alexey.K (R2SI)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\320\230\320\273\321\214\320\264"
                        "\320\260\321\200.\320\234 (R9JAU)</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#000000;\">\320\220\321\200\321\202\321\221\320\274.\320\241 (R4CAT)</span></p></body></html>", nullptr));
        pushButton->setText(QCoreApplication::translate("About", "\320\236\320\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class About: public Ui_About {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
