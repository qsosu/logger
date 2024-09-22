/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSettings;
    QAction *actionCallsigns;
    QAction *actionFlrig;
    QAction *actionExportAdif;
    QAction *actionSync;
    QAction *actionAbout;
    QAction *actionQsosuFaqLink;
    QAction *actionQsosuLink;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_19;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_12;
    QComboBox *stationCallsignCombo;
    QLabel *label_13;
    QComboBox *operatorCombo;
    QLabel *label_14;
    QLineEdit *qthlocEdit;
    QLabel *label_16;
    QLineEdit *rdaEdit;
    QSpacerItem *horizontalSpacer_6;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *callsignLabel;
    QLineEdit *callInput;
    QLabel *label_2;
    QDateEdit *dateInput;
    QLabel *label_3;
    QLineEdit *timeInput;
    QCheckBox *showCurrentTime;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_4;
    QComboBox *bandCombo;
    QLabel *label_5;
    QLineEdit *freqInput;
    QLabel *label_6;
    QComboBox *modeCombo;
    QLabel *label_7;
    QLineEdit *rstrInput;
    QLabel *label_8;
    QLineEdit *rstsInput;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_11;
    QLineEdit *nameInput;
    QLabel *label_10;
    QLineEdit *qthInput;
    QLabel *label_9;
    QLineEdit *gridsquareInput;
    QLabel *label_20;
    QLineEdit *cntyInput;
    QLabel *label_15;
    QLineEdit *commentInput;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_8;
    QPushButton *saveQsoButton;
    QPushButton *clearQsoButton;
    QPushButton *refreshButton;
    QSpacerItem *horizontalSpacer_9;
    QTableView *tableView;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_17;
    QLabel *qsosuLabel;
    QLabel *label_18;
    QLabel *udpserverLabel;
    QLabel *label;
    QLabel *flrigLabel;
    QMenuBar *menubar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *menu_3;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(996, 600);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName("actionSettings");
        actionCallsigns = new QAction(MainWindow);
        actionCallsigns->setObjectName("actionCallsigns");
        actionFlrig = new QAction(MainWindow);
        actionFlrig->setObjectName("actionFlrig");
        actionFlrig->setCheckable(true);
        actionExportAdif = new QAction(MainWindow);
        actionExportAdif->setObjectName("actionExportAdif");
        actionSync = new QAction(MainWindow);
        actionSync->setObjectName("actionSync");
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName("actionAbout");
        actionQsosuFaqLink = new QAction(MainWindow);
        actionQsosuFaqLink->setObjectName("actionQsosuFaqLink");
        actionQsosuLink = new QAction(MainWindow);
        actionQsosuLink->setObjectName("actionQsosuLink");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);

        label_19 = new QLabel(centralwidget);
        label_19->setObjectName("label_19");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_19->sizePolicy().hasHeightForWidth());
        label_19->setSizePolicy(sizePolicy);
        label_19->setMinimumSize(QSize(100, 40));
        label_19->setMaximumSize(QSize(100, 40));
        label_19->setPixmap(QPixmap(QString::fromUtf8(":/resources/images/logo_mini.svg")));
        label_19->setScaledContents(true);
        label_19->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_6->addWidget(label_19);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName("groupBox_2");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(3);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy1);
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalLayout_4->setContentsMargins(3, 3, 3, 3);
        label_12 = new QLabel(groupBox_2);
        label_12->setObjectName("label_12");

        horizontalLayout_4->addWidget(label_12);

        stationCallsignCombo = new QComboBox(groupBox_2);
        stationCallsignCombo->setObjectName("stationCallsignCombo");

        horizontalLayout_4->addWidget(stationCallsignCombo);

        label_13 = new QLabel(groupBox_2);
        label_13->setObjectName("label_13");

        horizontalLayout_4->addWidget(label_13);

        operatorCombo = new QComboBox(groupBox_2);
        operatorCombo->setObjectName("operatorCombo");

        horizontalLayout_4->addWidget(operatorCombo);

        label_14 = new QLabel(groupBox_2);
        label_14->setObjectName("label_14");

        horizontalLayout_4->addWidget(label_14);

        qthlocEdit = new QLineEdit(groupBox_2);
        qthlocEdit->setObjectName("qthlocEdit");
        qthlocEdit->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(qthlocEdit->sizePolicy().hasHeightForWidth());
        qthlocEdit->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(qthlocEdit);

        label_16 = new QLabel(groupBox_2);
        label_16->setObjectName("label_16");

        horizontalLayout_4->addWidget(label_16);

        rdaEdit = new QLineEdit(groupBox_2);
        rdaEdit->setObjectName("rdaEdit");
        rdaEdit->setEnabled(true);
        sizePolicy2.setHeightForWidth(rdaEdit->sizePolicy().hasHeightForWidth());
        rdaEdit->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(rdaEdit);

        horizontalSpacer_6 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);


        horizontalLayout_6->addWidget(groupBox_2);


        verticalLayout_2->addLayout(horizontalLayout_6);

        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName("groupBox");
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(3, 3, 3, 3);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        callsignLabel = new QLabel(groupBox);
        callsignLabel->setObjectName("callsignLabel");

        horizontalLayout->addWidget(callsignLabel);

        callInput = new QLineEdit(groupBox);
        callInput->setObjectName("callInput");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(callInput->sizePolicy().hasHeightForWidth());
        callInput->setSizePolicy(sizePolicy3);
        callInput->setMinimumSize(QSize(0, 0));
        callInput->setCursorPosition(0);

        horizontalLayout->addWidget(callInput);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        horizontalLayout->addWidget(label_2);

        dateInput = new QDateEdit(groupBox);
        dateInput->setObjectName("dateInput");
        dateInput->setMinimumSize(QSize(130, 0));
        dateInput->setCalendarPopup(true);
        dateInput->setDate(QDate(2024, 9, 17));

        horizontalLayout->addWidget(dateInput);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        horizontalLayout->addWidget(label_3);

        timeInput = new QLineEdit(groupBox);
        timeInput->setObjectName("timeInput");
        sizePolicy3.setHeightForWidth(timeInput->sizePolicy().hasHeightForWidth());
        timeInput->setSizePolicy(sizePolicy3);
        timeInput->setMinimumSize(QSize(0, 0));
        timeInput->setMaxLength(8);

        horizontalLayout->addWidget(timeInput);

        showCurrentTime = new QCheckBox(groupBox);
        showCurrentTime->setObjectName("showCurrentTime");

        horizontalLayout->addWidget(showCurrentTime);

        horizontalSpacer = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        horizontalLayout_2->addWidget(label_4);

        bandCombo = new QComboBox(groupBox);
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->addItem(QString());
        bandCombo->setObjectName("bandCombo");
        sizePolicy3.setHeightForWidth(bandCombo->sizePolicy().hasHeightForWidth());
        bandCombo->setSizePolicy(sizePolicy3);
        bandCombo->setMinimumSize(QSize(0, 0));

        horizontalLayout_2->addWidget(bandCombo);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        horizontalLayout_2->addWidget(label_5);

        freqInput = new QLineEdit(groupBox);
        freqInput->setObjectName("freqInput");
        sizePolicy3.setHeightForWidth(freqInput->sizePolicy().hasHeightForWidth());
        freqInput->setSizePolicy(sizePolicy3);
        freqInput->setMinimumSize(QSize(0, 0));

        horizontalLayout_2->addWidget(freqInput);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        horizontalLayout_2->addWidget(label_6);

        modeCombo = new QComboBox(groupBox);
        modeCombo->setObjectName("modeCombo");
        sizePolicy3.setHeightForWidth(modeCombo->sizePolicy().hasHeightForWidth());
        modeCombo->setSizePolicy(sizePolicy3);
        modeCombo->setMinimumSize(QSize(0, 22));
        modeCombo->setMaximumSize(QSize(16777215, 100));
        modeCombo->setAutoFillBackground(false);
        modeCombo->setStyleSheet(QString::fromUtf8(""));
        modeCombo->setInputMethodHints(Qt::InputMethodHint::ImhNone);
        modeCombo->setMaxVisibleItems(50);
        modeCombo->setMaxCount(210);
        modeCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
        modeCombo->setMinimumContentsLength(10);
        modeCombo->setFrame(true);
        modeCombo->setModelColumn(0);

        horizontalLayout_2->addWidget(modeCombo);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        horizontalLayout_2->addWidget(label_7);

        rstrInput = new QLineEdit(groupBox);
        rstrInput->setObjectName("rstrInput");
        sizePolicy3.setHeightForWidth(rstrInput->sizePolicy().hasHeightForWidth());
        rstrInput->setSizePolicy(sizePolicy3);
        rstrInput->setMinimumSize(QSize(30, 0));
        rstrInput->setMaxLength(4);

        horizontalLayout_2->addWidget(rstrInput);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        horizontalLayout_2->addWidget(label_8);

        rstsInput = new QLineEdit(groupBox);
        rstsInput->setObjectName("rstsInput");
        sizePolicy3.setHeightForWidth(rstsInput->sizePolicy().hasHeightForWidth());
        rstsInput->setSizePolicy(sizePolicy3);
        rstsInput->setMinimumSize(QSize(30, 0));
        rstsInput->setMaxLength(4);

        horizontalLayout_2->addWidget(rstsInput);

        horizontalSpacer_3 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_11 = new QLabel(groupBox);
        label_11->setObjectName("label_11");

        horizontalLayout_3->addWidget(label_11);

        nameInput = new QLineEdit(groupBox);
        nameInput->setObjectName("nameInput");
        sizePolicy2.setHeightForWidth(nameInput->sizePolicy().hasHeightForWidth());
        nameInput->setSizePolicy(sizePolicy2);
        nameInput->setMinimumSize(QSize(0, 0));

        horizontalLayout_3->addWidget(nameInput);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName("label_10");

        horizontalLayout_3->addWidget(label_10);

        qthInput = new QLineEdit(groupBox);
        qthInput->setObjectName("qthInput");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy4.setHorizontalStretch(2);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(qthInput->sizePolicy().hasHeightForWidth());
        qthInput->setSizePolicy(sizePolicy4);
        qthInput->setMinimumSize(QSize(0, 0));

        horizontalLayout_3->addWidget(qthInput);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName("label_9");

        horizontalLayout_3->addWidget(label_9);

        gridsquareInput = new QLineEdit(groupBox);
        gridsquareInput->setObjectName("gridsquareInput");
        sizePolicy3.setHeightForWidth(gridsquareInput->sizePolicy().hasHeightForWidth());
        gridsquareInput->setSizePolicy(sizePolicy3);
        gridsquareInput->setMinimumSize(QSize(0, 0));

        horizontalLayout_3->addWidget(gridsquareInput);

        label_20 = new QLabel(groupBox);
        label_20->setObjectName("label_20");

        horizontalLayout_3->addWidget(label_20);

        cntyInput = new QLineEdit(groupBox);
        cntyInput->setObjectName("cntyInput");

        horizontalLayout_3->addWidget(cntyInput);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName("label_15");

        horizontalLayout_3->addWidget(label_15);

        commentInput = new QLineEdit(groupBox);
        commentInput->setObjectName("commentInput");
        sizePolicy4.setHeightForWidth(commentInput->sizePolicy().hasHeightForWidth());
        commentInput->setSizePolicy(sizePolicy4);
        commentInput->setMinimumSize(QSize(0, 0));

        horizontalLayout_3->addWidget(commentInput);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        horizontalSpacer_8 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_8);

        saveQsoButton = new QPushButton(groupBox);
        saveQsoButton->setObjectName("saveQsoButton");
        saveQsoButton->setMinimumSize(QSize(0, 26));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resources/images/check.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        saveQsoButton->setIcon(icon);
        saveQsoButton->setIconSize(QSize(15, 15));

        horizontalLayout_7->addWidget(saveQsoButton);

        clearQsoButton = new QPushButton(groupBox);
        clearQsoButton->setObjectName("clearQsoButton");
        clearQsoButton->setMinimumSize(QSize(0, 26));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/resources/images/arrow-rotate-left.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        clearQsoButton->setIcon(icon1);
        clearQsoButton->setIconSize(QSize(15, 15));

        horizontalLayout_7->addWidget(clearQsoButton);

        refreshButton = new QPushButton(groupBox);
        refreshButton->setObjectName("refreshButton");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/resources/images/arrows-rotate.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        refreshButton->setIcon(icon2);
        refreshButton->setIconSize(QSize(15, 15));

        horizontalLayout_7->addWidget(refreshButton);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_9);


        verticalLayout->addLayout(horizontalLayout_7);


        verticalLayout_2->addWidget(groupBox);

        tableView = new QTableView(centralwidget);
        tableView->setObjectName("tableView");

        verticalLayout_2->addWidget(tableView);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        horizontalSpacer_4 = new QSpacerItem(0, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);

        label_17 = new QLabel(centralwidget);
        label_17->setObjectName("label_17");
        label_17->setEnabled(false);

        horizontalLayout_5->addWidget(label_17);

        qsosuLabel = new QLabel(centralwidget);
        qsosuLabel->setObjectName("qsosuLabel");
        qsosuLabel->setEnabled(false);

        horizontalLayout_5->addWidget(qsosuLabel);

        label_18 = new QLabel(centralwidget);
        label_18->setObjectName("label_18");

        horizontalLayout_5->addWidget(label_18);

        udpserverLabel = new QLabel(centralwidget);
        udpserverLabel->setObjectName("udpserverLabel");

        horizontalLayout_5->addWidget(udpserverLabel);

        label = new QLabel(centralwidget);
        label->setObjectName("label");

        horizontalLayout_5->addWidget(label);

        flrigLabel = new QLabel(centralwidget);
        flrigLabel->setObjectName("flrigLabel");

        horizontalLayout_5->addWidget(flrigLabel);


        verticalLayout_2->addLayout(horizontalLayout_5);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 996, 22));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        menu_2 = new QMenu(menubar);
        menu_2->setObjectName("menu_2");
        menu_3 = new QMenu(menubar);
        menu_3->setObjectName("menu_3");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu->menuAction());
        menubar->addAction(menu_2->menuAction());
        menubar->addAction(menu_3->menuAction());
        menu->addAction(actionSettings);
        menu->addAction(actionFlrig);
        menu_2->addAction(actionCallsigns);
        menu_2->addAction(actionExportAdif);
        menu_2->addAction(actionSync);
        menu_3->addAction(actionQsosuLink);
        menu_3->addAction(actionQsosuFaqLink);
        menu_3->addSeparator();
        menu_3->addAction(actionAbout);

        retranslateUi(MainWindow);

        modeCombo->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionSettings->setText(QCoreApplication::translate("MainWindow", "\320\235\320\260\321\201\321\202\321\200\320\276\320\271\320\272\320\270 \320\277\321\200\320\276\320\263\321\200\320\260\320\274\320\274\321\213", nullptr));
        actionCallsigns->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\320\267\321\213\320\262\320\275\321\213\320\265", nullptr));
        actionFlrig->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\320\264\320\272\320\273\321\216\321\207\320\270\321\202\321\214 FLRIG", nullptr));
        actionExportAdif->setText(QCoreApplication::translate("MainWindow", "\320\255\320\272\321\201\320\277\320\276\321\200\321\202 \320\262\321\201\320\265\320\263\320\276 \320\266\321\203\321\200\320\275\320\260\320\273\320\260 \320\262 ADIF", nullptr));
        actionSync->setText(QCoreApplication::translate("MainWindow", "\320\241\320\270\320\275\321\205\321\200\320\276\320\275\320\270\320\267\320\270\321\200\320\276\320\262\320\260\321\202\321\214 \320\266\321\203\321\200\320\275\320\260\320\273", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "\320\236 \320\277\321\200\320\276\320\263\321\200\320\260\320\274\320\274\320\265", nullptr));
        actionQsosuFaqLink->setText(QCoreApplication::translate("MainWindow", "FAQ", nullptr));
        actionQsosuLink->setText(QCoreApplication::translate("MainWindow", "\320\241\321\202\321\200\320\260\320\275\320\270\321\206\320\260 \321\201\320\265\321\200\320\262\320\270\321\201\320\260 QSO.SU", nullptr));
        label_19->setText(QString());
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 QSO", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\320\267\321\213\320\262\320\275\320\276\320\271 \321\201\321\202\320\260\320\275\321\206\320\270\320\270", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "\320\236\320\277\320\265\321\200\320\260\321\202\320\276\321\200", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "QTH \320\273\320\276\320\272\320\260\321\202\320\276\321\200", nullptr));
        label_16->setText(QCoreApplication::translate("MainWindow", "RDA/CNTY", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "\320\235\320\276\320\262\320\276\320\265 QSO", nullptr));
        callsignLabel->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\320\267\321\213\320\262\320\275\320\276\320\271", nullptr));
        callInput->setInputMask(QString());
        callInput->setText(QString());
        label_2->setText(QCoreApplication::translate("MainWindow", "\320\224\320\260\321\202\320\260", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\320\222\321\200\320\265\320\274\321\217 \320\262 UTC", nullptr));
#if QT_CONFIG(accessibility)
        timeInput->setAccessibleDescription(QCoreApplication::translate("MainWindow", "99:99:99", nullptr));
#endif // QT_CONFIG(accessibility)
        timeInput->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        showCurrentTime->setText(QCoreApplication::translate("MainWindow", "\320\242\320\265\320\272\321\203\321\211\320\265\320\265 \320\262\321\200\320\265\320\274\321\217", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "\320\224\320\270\320\260\320\277\320\260\320\267\320\276\320\275", nullptr));
        bandCombo->setItemText(0, QCoreApplication::translate("MainWindow", "2190M", nullptr));
        bandCombo->setItemText(1, QCoreApplication::translate("MainWindow", "630M", nullptr));
        bandCombo->setItemText(2, QCoreApplication::translate("MainWindow", "160M", nullptr));
        bandCombo->setItemText(3, QCoreApplication::translate("MainWindow", "80M", nullptr));
        bandCombo->setItemText(4, QCoreApplication::translate("MainWindow", "60M", nullptr));
        bandCombo->setItemText(5, QCoreApplication::translate("MainWindow", "40M", nullptr));
        bandCombo->setItemText(6, QCoreApplication::translate("MainWindow", "30M", nullptr));
        bandCombo->setItemText(7, QCoreApplication::translate("MainWindow", "20M", nullptr));
        bandCombo->setItemText(8, QCoreApplication::translate("MainWindow", "17M", nullptr));
        bandCombo->setItemText(9, QCoreApplication::translate("MainWindow", "15M", nullptr));
        bandCombo->setItemText(10, QCoreApplication::translate("MainWindow", "12M", nullptr));
        bandCombo->setItemText(11, QCoreApplication::translate("MainWindow", "10M", nullptr));
        bandCombo->setItemText(12, QCoreApplication::translate("MainWindow", "6M", nullptr));
        bandCombo->setItemText(13, QCoreApplication::translate("MainWindow", "4M", nullptr));
        bandCombo->setItemText(14, QCoreApplication::translate("MainWindow", "2M", nullptr));
        bandCombo->setItemText(15, QCoreApplication::translate("MainWindow", "1.25M", nullptr));
        bandCombo->setItemText(16, QCoreApplication::translate("MainWindow", "70CM", nullptr));
        bandCombo->setItemText(17, QCoreApplication::translate("MainWindow", "33CM", nullptr));
        bandCombo->setItemText(18, QCoreApplication::translate("MainWindow", "21CM", nullptr));
        bandCombo->setItemText(19, QCoreApplication::translate("MainWindow", "23CM", nullptr));
        bandCombo->setItemText(20, QCoreApplication::translate("MainWindow", "13CM", nullptr));
        bandCombo->setItemText(21, QCoreApplication::translate("MainWindow", "9CM", nullptr));
        bandCombo->setItemText(22, QCoreApplication::translate("MainWindow", "6CM", nullptr));
        bandCombo->setItemText(23, QCoreApplication::translate("MainWindow", "3CM", nullptr));
        bandCombo->setItemText(24, QCoreApplication::translate("MainWindow", "1.25CM", nullptr));

        bandCombo->setCurrentText(QCoreApplication::translate("MainWindow", "2190M", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "\320\247\320\260\321\201\321\202\320\276\321\202\320\260, \320\234\320\223\321\206", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "\320\234\320\276\320\264\321\203\320\273\321\217\321\206\320\270\321\217", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "RST \320\277\321\200\320\270\320\275.", nullptr));
        rstrInput->setText(QCoreApplication::translate("MainWindow", "59", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "RST \320\276\321\202\320\277\321\200.", nullptr));
        rstsInput->setText(QCoreApplication::translate("MainWindow", "59", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "\320\230\320\274\321\217", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "QTH", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "QTH \320\273\320\276\320\272\320\260\321\202\320\276\321\200", nullptr));
        label_20->setText(QCoreApplication::translate("MainWindow", "RDA/CNTY", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow", "\320\232\320\276\320\274\320\274\320\265\320\275\321\202\320\260\321\200\320\270\320\271", nullptr));
        saveQsoButton->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214 QSO", nullptr));
        clearQsoButton->setText(QCoreApplication::translate("MainWindow", "\320\236\321\207\320\270\321\201\321\202\320\270\321\202\321\214", nullptr));
        refreshButton->setText(QCoreApplication::translate("MainWindow", "\320\236\320\261\320\275\320\276\320\262\320\270\321\202\321\214 \321\201\320\277\320\270\321\201\320\276\320\272 QSO", nullptr));
        label_17->setText(QCoreApplication::translate("MainWindow", "\320\241\320\265\321\200\320\262\320\270\321\201 QSO.SU:", nullptr));
        qsosuLabel->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        label_18->setText(QCoreApplication::translate("MainWindow", "UDP \321\201\320\265\321\200\320\262\320\265\321\200:", nullptr));
        udpserverLabel->setText(QCoreApplication::translate("MainWindow", "\320\235\320\265 \320\267\320\260\320\277\321\203\321\211\320\265\320\275", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "FLRIG:", nullptr));
        flrigLabel->setText(QCoreApplication::translate("MainWindow", "\320\236\321\202\320\272\320\273\321\216\321\207\320\265\320\275", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\320\236\320\277\321\206\320\270\320\270", nullptr));
        menu_2->setTitle(QCoreApplication::translate("MainWindow", "\320\226\321\203\321\200\320\275\320\260\320\273", nullptr));
        menu_3->setTitle(QCoreApplication::translate("MainWindow", "\320\241\320\277\321\200\320\260\320\262\320\272\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
