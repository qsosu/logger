/********************************************************************************
** Form generated from reading UI file 'settings.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Settings
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QLineEdit *accessToken;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QCheckBox *udpServerEnableCheckbox;
    QLabel *label;
    QSpinBox *udpServerPort;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QLineEdit *flrigHost;
    QLabel *label_4;
    QSpinBox *flrigPort;
    QLabel *label_5;
    QSpinBox *flrigPeriod;
    QSpacerItem *horizontalSpacer_4;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *qrzruEnable;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_6;
    QLineEdit *qrzruLogin;
    QLabel *label_7;
    QLineEdit *qrzruPassword;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_9;
    QSpinBox *fontSize;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *saveButton;
    QPushButton *closeButton;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QDialog *Settings)
    {
        if (Settings->objectName().isEmpty())
            Settings->setObjectName("Settings");
        Settings->resize(608, 374);
        verticalLayout = new QVBoxLayout(Settings);
        verticalLayout->setObjectName("verticalLayout");
        groupBox_2 = new QGroupBox(Settings);
        groupBox_2->setObjectName("groupBox_2");
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalLayout_4->setContentsMargins(3, 3, 3, 3);
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName("label_2");

        horizontalLayout_4->addWidget(label_2);

        accessToken = new QLineEdit(groupBox_2);
        accessToken->setObjectName("accessToken");

        horizontalLayout_4->addWidget(accessToken);


        verticalLayout->addWidget(groupBox_2);

        groupBox = new QGroupBox(Settings);
        groupBox->setObjectName("groupBox");
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(3, 3, 3, 3);
        udpServerEnableCheckbox = new QCheckBox(groupBox);
        udpServerEnableCheckbox->setObjectName("udpServerEnableCheckbox");

        horizontalLayout->addWidget(udpServerEnableCheckbox);

        label = new QLabel(groupBox);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        udpServerPort = new QSpinBox(groupBox);
        udpServerPort->setObjectName("udpServerPort");
        udpServerPort->setMinimum(1);
        udpServerPort->setMaximum(65535);
        udpServerPort->setValue(2237);

        horizontalLayout->addWidget(udpServerPort);

        horizontalSpacer = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addWidget(groupBox);

        groupBox_3 = new QGroupBox(Settings);
        groupBox_3->setObjectName("groupBox_3");
        horizontalLayout_5 = new QHBoxLayout(groupBox_3);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        horizontalLayout_5->setContentsMargins(3, 3, 3, 3);
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName("label_3");

        horizontalLayout_5->addWidget(label_3);

        flrigHost = new QLineEdit(groupBox_3);
        flrigHost->setObjectName("flrigHost");

        horizontalLayout_5->addWidget(flrigHost);

        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName("label_4");

        horizontalLayout_5->addWidget(label_4);

        flrigPort = new QSpinBox(groupBox_3);
        flrigPort->setObjectName("flrigPort");
        flrigPort->setMinimum(1);
        flrigPort->setMaximum(65535);
        flrigPort->setValue(12345);

        horizontalLayout_5->addWidget(flrigPort);

        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName("label_5");

        horizontalLayout_5->addWidget(label_5);

        flrigPeriod = new QSpinBox(groupBox_3);
        flrigPeriod->setObjectName("flrigPeriod");
        flrigPeriod->setMinimum(200);
        flrigPeriod->setMaximum(5000);
        flrigPeriod->setValue(500);

        horizontalLayout_5->addWidget(flrigPeriod);

        horizontalSpacer_4 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);


        verticalLayout->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(Settings);
        groupBox_4->setObjectName("groupBox_4");
        horizontalLayout_2 = new QHBoxLayout(groupBox_4);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(3, 3, 3, 3);
        qrzruEnable = new QCheckBox(groupBox_4);
        qrzruEnable->setObjectName("qrzruEnable");

        horizontalLayout_2->addWidget(qrzruEnable);

        horizontalSpacer_5 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);

        label_6 = new QLabel(groupBox_4);
        label_6->setObjectName("label_6");

        horizontalLayout_2->addWidget(label_6);

        qrzruLogin = new QLineEdit(groupBox_4);
        qrzruLogin->setObjectName("qrzruLogin");

        horizontalLayout_2->addWidget(qrzruLogin);

        label_7 = new QLabel(groupBox_4);
        label_7->setObjectName("label_7");

        horizontalLayout_2->addWidget(label_7);

        qrzruPassword = new QLineEdit(groupBox_4);
        qrzruPassword->setObjectName("qrzruPassword");

        horizontalLayout_2->addWidget(qrzruPassword);


        verticalLayout->addWidget(groupBox_4);

        groupBox_5 = new QGroupBox(Settings);
        groupBox_5->setObjectName("groupBox_5");
        horizontalLayout_6 = new QHBoxLayout(groupBox_5);
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        horizontalLayout_6->setContentsMargins(3, 3, 3, 3);
        label_9 = new QLabel(groupBox_5);
        label_9->setObjectName("label_9");

        horizontalLayout_6->addWidget(label_9);

        fontSize = new QSpinBox(groupBox_5);
        fontSize->setObjectName("fontSize");
        fontSize->setMinimum(8);
        fontSize->setMaximum(16);

        horizontalLayout_6->addWidget(fontSize);

        horizontalSpacer_6 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_6);


        verticalLayout->addWidget(groupBox_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalSpacer_2 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        saveButton = new QPushButton(Settings);
        saveButton->setObjectName("saveButton");

        horizontalLayout_3->addWidget(saveButton);

        closeButton = new QPushButton(Settings);
        closeButton->setObjectName("closeButton");

        horizontalLayout_3->addWidget(closeButton);

        horizontalSpacer_3 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(Settings);

        QMetaObject::connectSlotsByName(Settings);
    } // setupUi

    void retranslateUi(QDialog *Settings)
    {
        Settings->setWindowTitle(QCoreApplication::translate("Settings", "Dialog", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("Settings", "QSO.SU API", nullptr));
        label_2->setText(QCoreApplication::translate("Settings", "Access token", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Settings", "\320\240\320\260\320\261\320\276\321\202\320\260 \320\277\320\276 \321\201\320\265\321\202\320\270", nullptr));
        udpServerEnableCheckbox->setText(QCoreApplication::translate("Settings", "\320\237\321\200\320\270\320\275\320\270\320\274\320\260\321\202\321\214 QSO \320\277\320\276 \321\201\320\265\321\202\320\270", nullptr));
        label->setText(QCoreApplication::translate("Settings", "UDP \320\277\320\276\321\200\321\202", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("Settings", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \320\277\320\276\320\264\320\272\320\273\321\216\321\207\320\265\320\275\320\270\321\217 \320\272 FLRIG (XML-RPC)", nullptr));
        label_3->setText(QCoreApplication::translate("Settings", "\320\220\320\264\321\200\320\265\321\201 \321\205\320\276\321\201\321\202\320\260", nullptr));
        flrigHost->setText(QCoreApplication::translate("Settings", "127.0.0.1", nullptr));
        label_4->setText(QCoreApplication::translate("Settings", "\320\237\320\276\321\200\321\202", nullptr));
        label_5->setText(QCoreApplication::translate("Settings", "\320\237\320\265\321\200\320\270\320\276\320\264 \320\276\320\261\320\275\320\276\320\262\320\273\320\265\320\275\320\270\321\217, \320\274\321\201", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("Settings", "QRZ.RU \320\272\320\276\320\273\320\273\320\261\321\203\320\272", nullptr));
        qrzruEnable->setText(QCoreApplication::translate("Settings", "\320\230\321\201\320\277\320\276\320\273\321\214\320\267\320\276\320\262\320\260\321\202\321\214", nullptr));
        label_6->setText(QCoreApplication::translate("Settings", "\320\233\320\276\320\263\320\270\320\275", nullptr));
        label_7->setText(QCoreApplication::translate("Settings", "\320\237\320\260\321\200\320\276\320\273\321\214", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("Settings", "\320\222\320\275\320\265\321\210\320\275\320\270\320\271 \320\262\320\270\320\264", nullptr));
        label_9->setText(QCoreApplication::translate("Settings", "\320\240\320\260\320\267\320\274\320\265\321\200 \321\210\321\200\320\270\321\204\321\202\320\260", nullptr));
        saveButton->setText(QCoreApplication::translate("Settings", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214 \320\275\320\260\321\201\321\202\321\200\320\276\320\271\320\272\320\270", nullptr));
        closeButton->setText(QCoreApplication::translate("Settings", "\320\227\320\260\320\272\321\200\321\213\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Settings: public Ui_Settings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGS_H
