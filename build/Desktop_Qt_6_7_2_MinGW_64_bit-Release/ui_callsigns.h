/********************************************************************************
** Form generated from reading UI file 'callsigns.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CALLSIGNS_H
#define UI_CALLSIGNS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Callsigns
{
public:
    QVBoxLayout *verticalLayout;
    QTableView *callTable;
    QHBoxLayout *horizontalLayout;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *updateFromQsosu;

    void setupUi(QDialog *Callsigns)
    {
        if (Callsigns->objectName().isEmpty())
            Callsigns->setObjectName("Callsigns");
        Callsigns->resize(655, 296);
        verticalLayout = new QVBoxLayout(Callsigns);
        verticalLayout->setObjectName("verticalLayout");
        callTable = new QTableView(Callsigns);
        callTable->setObjectName("callTable");

        verticalLayout->addWidget(callTable);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        addButton = new QPushButton(Callsigns);
        addButton->setObjectName("addButton");

        horizontalLayout->addWidget(addButton);

        removeButton = new QPushButton(Callsigns);
        removeButton->setObjectName("removeButton");

        horizontalLayout->addWidget(removeButton);

        saveButton = new QPushButton(Callsigns);
        saveButton->setObjectName("saveButton");

        horizontalLayout->addWidget(saveButton);

        cancelButton = new QPushButton(Callsigns);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout->addWidget(cancelButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        updateFromQsosu = new QPushButton(Callsigns);
        updateFromQsosu->setObjectName("updateFromQsosu");

        horizontalLayout->addWidget(updateFromQsosu);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(Callsigns);

        QMetaObject::connectSlotsByName(Callsigns);
    } // setupUi

    void retranslateUi(QDialog *Callsigns)
    {
        Callsigns->setWindowTitle(QCoreApplication::translate("Callsigns", "Dialog", nullptr));
        addButton->setText(QCoreApplication::translate("Callsigns", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \320\275\320\276\320\262\321\213\320\271", nullptr));
        removeButton->setText(QCoreApplication::translate("Callsigns", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214", nullptr));
        saveButton->setText(QCoreApplication::translate("Callsigns", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
        cancelButton->setText(QCoreApplication::translate("Callsigns", "\320\236\321\202\320\274\320\265\320\275\320\270\321\202\321\214", nullptr));
        updateFromQsosu->setText(QCoreApplication::translate("Callsigns", "\320\236\320\261\320\275\320\276\320\262\320\270\321\202\321\214 \321\201 QSO.SU", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Callsigns: public Ui_Callsigns {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CALLSIGNS_H
