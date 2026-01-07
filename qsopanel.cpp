/**********************************************************************************************************
Description :  Implementation of the QSOPanel class, which provides a flexible and dockable user interface
            :  panel for entering and editing QSO data. The panel can be docked at the top of the main
            :  window or detached into a floating window with support for manual resizing and drag-and-drop
            :  docking.
Version     :  1.2.0
Date        :  01.09.2025
Author      :  R9JAU
Comments    :  - Built using FlowLayout-based containers for adaptive and responsive placement of input fields.
            :  - Provides validators for input fields (callsigns, RST values, locators).
            :  - Fully dockable with visual dock indicators (QRubberBand).
***********************************************************************************************************/

#include "qsopanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QTimer>
#include <QCompleter>
#include <QDate>
#include <QTime>
#include <QSpacerItem>
#include <QScreen>



QSOPanel::QSOPanel(QMainWindow *mainWindow, Settings *settings, cat_Interface *cat, QWidget *parent)
    : QWidget(parent),
      m_mainWindow(mainWindow),
      m_docked(true),
      m_dragging(false),
      m_dockPos(Top),
      m_resizing(false),
      m_resizeTop(false), m_resizeBottom(false),
      m_resizeLeft(false), m_resizeRight(false),
      m_rubberBand(nullptr)
{
    setMouseTracking(true);
    ui.setupUi(this);
    this->settings = settings;
    this->CAT = cat;

    // Чтобы при запуске не раздувалось
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMinimumSize(400, 250);  // минимальный размер по умолчанию
    resize(500, 250);          // стартовый размер

    ui.groupBox1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    ui.groupBox2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    // GroupBox 1
    QVBoxLayout *boxLayout1 = new QVBoxLayout;
    boxLayout1->setContentsMargins(0, 0, 0, 0);
    boxLayout1->setSpacing(5);
    //boxLayout1->addStretch();

    // FlowLayout1
    QWidget *flowContainer1 = new QWidget;
    flowLayout1 = new FlowLayout(flowContainer1, 2, 6, 6);
    flowContainer1->setLayout(flowLayout1); // Только один setLayout
    flowContainer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // FlowLayout2
    QWidget *flowContainer2 = new QWidget;
    flowLayout2 = new FlowLayout(flowContainer2, 2, 6, 6);
    flowContainer2->setLayout(flowLayout2);
    flowContainer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    boxLayout1->addWidget(flowContainer1);
    boxLayout1->addWidget(flowContainer2);

    ui.groupBox1->setLayout(boxLayout1);
    ui.groupBox1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Заполнение GroupBox1
    stationCallsign = new QComboBox;
    stationCallsign->setMinimumWidth(50);
    flowLayout1->addWidget(makeInputPair(tr("Позывной станции:"), stationCallsign));
    operatorCallsign = new QComboBox;
    operatorCallsign->setMinimumWidth(50);
    flowLayout1->addWidget(makeInputPair(tr("Позывной оператора:"), operatorCallsign));
    DateEdit = new QDateEdit;
    DateEdit->setCalendarPopup(true);
    flowLayout1->addWidget(makeInputPair(tr("Дата:"), DateEdit));
    TimeEdit = new QTimeEdit;
    TimeEdit->setDisplayFormat("HH:mm:ss");
    ShowCurrentTime = new QCheckBox(tr("Реальное время"));
    flowLayout1->addWidget(makeLabelWidgetPair(tr("Время UTC:"), TimeEdit, ShowCurrentTime));

    BandCombo = new QComboBox;
    flowLayout2->addWidget(makeInputPair(tr("Диапазон:"), BandCombo));
    ModeCombo = new QComboBox;
    ModeCombo->setMinimumWidth(100);
    flowLayout2->addWidget(makeInputPair(tr("Модуляция:"), ModeCombo));
    FreqInput = new QLineEdit;
    flowLayout2->addWidget(makeInputPair(tr("Частота, МГц:"), FreqInput));
    QTHLocEdit = new QLineEdit;
    flowLayout2->addWidget(makeInputPair(tr("QTH локатор:"), QTHLocEdit));
    RDAEdit = new QLineEdit;
    flowLayout2->addWidget(makeInputPair("RDA/CNTY:", RDAEdit));

    // GroupBox 2
    QVBoxLayout *boxLayout2 = new QVBoxLayout;
    boxLayout2->setContentsMargins(0, 2, 0, 2);
    boxLayout2->setSpacing(2);
    //boxLayout2->addStretch();

    // FlowLayout3
    QWidget *flowContainer3 = new QWidget;
    flowLayout3 = new FlowLayout(flowContainer3, 2, 6, 6);
    flowContainer3->setLayout(flowLayout3);
    flowContainer3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // FlowLayout4
    QWidget *flowContainer4 = new QWidget;
    flowLayout4 = new FlowLayout(flowContainer4, 2, 6, 6);
    flowContainer4->setLayout(flowLayout4);
    flowContainer4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // FlowLayout5
    QWidget *flowContainer5 = new QWidget;
    flowLayout5 = new FlowLayout(flowContainer5, 2, 6, 6);
    flowContainer5->setLayout(flowLayout5);
    flowContainer5->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    boxLayout2->addWidget(flowContainer3);
    boxLayout2->addWidget(flowContainer4);
    boxLayout2->addWidget(flowContainer5);

    ui.groupBox2->setLayout(boxLayout2);
    ui.groupBox2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Заполнение GroupBox2
    CallInput = new QLineEdit;
    CallInput->setFixedHeight(30);
    QFont font = CallInput->font();
    font.setPointSize(14); // размер шрифта
    CallInput->setFont(font);
    flowLayout3->addWidget(makeInputPair(tr("Позывной:"), CallInput));

    countryFlag = new QLabel;
    countryFlag->setFixedSize(30, 22);
    countryFlag->setScaledContents(true);
    countryFlag->setAlignment(Qt::AlignCenter);
    countryFlag->setContentsMargins(0, 5, 0, 0);
    countryFlag->setVisible(false);

    countryName = new QLabel;
    countryName->setVisible(false);
    countryName->setStyleSheet(
        "color: rgb(25, 135, 84);"
        "font-size: 9pt;"
        "font-weight: 500;"
    );
    countryName->setContentsMargins(0, 4, 0, 0);
    flowLayout3->addWidget(makeWidgetPair(countryFlag, countryName));

    NameInput = new QLineEdit;
    flowLayout3->addWidget(makeInputPair(tr("Имя:"), NameInput));
    QTHInput = new QLineEdit;
    flowLayout3->addWidget(makeInputPair(tr("QTH:"), QTHInput));
    GridSquareInput = new QLineEdit;
    flowLayout3->addWidget(makeInputPair(tr("Локатор:"), GridSquareInput));
    CNTYInput = new QLineEdit;
    flowLayout3->addWidget(makeInputPair("RDA/CNTY:", CNTYInput));

    RstsInput = new QLineEdit;
    flowLayout4->addWidget(makeInputPair(tr("RST отпр.:"), RstsInput));
    RstrInput = new QLineEdit;
    flowLayout4->addWidget(makeInputPair(tr("RST прин.:"), RstrInput));
    CommentInput = new QLineEdit;
    flowLayout4->addWidget(makeInputPair(tr("Комментарий:"), CommentInput));

    QSOSUserIcon = new QLabel;
    QSOSUserIcon->setVisible(false);
    QSOSUserLabel = new QLabel(tr("Не пользователь QSO.SU"));
    QSOSUserLabel->setVisible(false);
    flowLayout5->addWidget(makeWidgetPair(QSOSUserIcon, QSOSUserLabel));

    UserSRRIcon = new QLabel;
    UserSRRIcon->setVisible(false);
    UserSRRLabel = new QLabel(tr("Член СРР"));
    UserSRRLabel->setVisible(false);
    flowLayout5->addWidget(makeWidgetPair(UserSRRIcon, UserSRRLabel));

    connect(ui.closeBtn, &QPushButton::clicked, this, &QSOPanel::toggleDock);
    connect(CallInput, SIGNAL(textEdited(const QString&)), this, SLOT(CallsignToUppercase(const QString&)));
    connect(stationCallsign, SIGNAL(currentIndexChanged(int)), this, SLOT(on_StationCallsignCurrentIndexChanged(int)));
    connect(operatorCallsign, SIGNAL(currentIndexChanged(int)), this, SLOT(on_OperatorCallsignCurrentIndexChanged(int)));
    connect(BandCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_BandComboCurrentIndexChanged(int)));
    connect(BandCombo, SIGNAL(currentTextChanged(const QString&)), this, SLOT(on_BandComboCurrentTextChanged(const QString&)));
    connect(ModeCombo, SIGNAL(currentTextChanged(const QString&)), this, SLOT(on_ModeComboCurrentTextChanged(const QString&)));
    connect(FreqInput, SIGNAL(textChanged(const QString&)), this, SLOT(on_FreqInputTextChanged(const QString&)));
    connect(FreqInput, SIGNAL(editingFinished()), this, SLOT(on_FreqInputEditingFinished()));
    connect(GridSquareInput, SIGNAL(textChanged(const QString&)), this, SLOT(on_GridSquareInputTextChanged(const QString&)));
    connect(CNTYInput, SIGNAL(textChanged(const QString&)), this, SLOT(on_CNTYInputTextChanged(const QString&)));
    connect(RstrInput, SIGNAL(editingFinished()), this, SLOT(on_RstrInputEditingFinished()));
    connect(RstsInput, SIGNAL(editingFinished()), this, SLOT(on_RstsInputEditingFinished()));

    ui.titleBar->installEventFilter(this);
    installEventFilter(this);

    //Настройка элементов интерфейса
    CallInput->setStyleSheet("font-weight: bold");
    CallInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9/]*$"), this));
    RstrInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[+-]?[0-9]*$"), this));
    RstsInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^[+-]?[0-9]*$"), this));
    GridSquareInput->setValidator(new QRegularExpressionValidator(QRegularExpression("^([a-zA-Z]{2})([0-9]{2})(((([a-zA-Z]{2}?)?)([0-9]{2}?)?)([a-zA-Z]{2}?)?)$/"), this));

    ModeCombo->setEditable(true); // Включаем встроенный QLineEdit
    ModeCombo->setInsertPolicy(QComboBox::NoInsert); // Отключаем вставку новых элементов из QLineEdit
    ModeCombo->completer()->setCompletionMode(QCompleter::CompletionMode::PopupCompletion); // устанавливаем модель автодополнения (по умолчанию стоит InlineCompletition)
    ModeCombo->completer()->setModelSorting(QCompleter::UnsortedModel);

    EverySecondTimer = new QTimer(this);
    EverySecondTimer->setInterval(1000);
    connect(EverySecondTimer, SIGNAL(timeout()), this, SLOT(UpdateFormDateTime()));

    ShowCurrentTime->setChecked(true);
    UpdateFormDateTime();
    EverySecondTimer->start();

    connect(ShowCurrentTime, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            UpdateFormDateTime();
            EverySecondTimer->start();
        } else {
            EverySecondTimer->stop();
        }
    });

    CallTypeTimer = new QTimer(this);
    CallTypeTimer->setSingleShot(true);
    CallTypeTimer->setInterval(1000);

    connect(CallTypeTimer, &QTimer::timeout, this, [=]() {
        emit findCallTimer();
    });

    BandCombo->setCurrentText(settings->lastBand);
    if(settings->lastMode == "") ModeCombo->setCurrentText("CW");
    else ModeCombo->setCurrentText(settings->lastMode);
    QTHLocEdit->setText(settings->lastLocator);
    RDAEdit->setText(settings->lastRDA);
    FreqInput->setText(settings->lastFrequence);
    RstrInput->setText(settings->lastRST_RCVD);
    RstsInput->setText(settings->lastRST_SENT);
    stationCallsign->setCurrentIndex(settings->lastCallsign);
    operatorCallsign->setCurrentIndex(settings->lastOperator);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_mainWindow->centralWidget()->layout());
    if (!layout) {
        layout = new QVBoxLayout(m_mainWindow->centralWidget());
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
    }
    layout->insertWidget(0, this);

    // фиксируем высоту при доке
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(250);

    // чтобы groupbox не раздувались
    for (auto *box : findChildren<QGroupBox*>()) {
        box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    }
    updateGeometry();
    parentWidget()->updateGeometry();

    QTimer::singleShot(0, this, [this]{
        dock();
    });
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *e = static_cast<QMouseEvent*>(event);
            if (e->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = e->globalPos() - frameGeometry().topLeft();
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            auto *e = static_cast<QMouseEvent*>(event);
            if (m_dragging) {
                if (m_docked && (e->globalPos() - m_dragPosition).manhattanLength() > 10) {
                    detach();
                    m_dragPosition = e->globalPos() - frameGeometry().topLeft();
                }
                if (!m_docked) {
                    move(e->globalPos() - m_dragPosition);
                    // показываем индикатор
                    showDockIndicator(detectDock());
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            if (m_dragging) {
                m_dragging = false;
                hideDockIndicator();
                if (!m_docked) tryDock();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::resizeEvent(QResizeEvent *event)
{
    int w = event->size().width();
    //int spacing = qMax(5, w / 50);
    int spacing = qBound(4, w / 80, 10);

    if (flowLayout1) flowLayout1->setSpacing(spacing);
    if (flowLayout2) flowLayout2->setSpacing(spacing);
    if (flowLayout3) flowLayout3->setSpacing(spacing);
    if (flowLayout4) flowLayout4->setSpacing(spacing);
    if (flowLayout5) flowLayout5->setSpacing(spacing);

    QWidget::resizeEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui.retranslateUi(this);

        // Статические элементы
        ui.closeBtn->setText(m_docked ? tr("][") : tr("[ ]"));
        ShowCurrentTime->setText(tr("Реальное время"));
        QSOSUserLabel->setText(tr("Не пользователь QSO.SU"));
        UserSRRLabel->setText(tr("Член СРР"));

        // Динамические элементы
        for (auto *w : flowLayoutWidgets) {
            if (auto lbl = qobject_cast<QLabel*>(w)) {
                if (dynamicLabelsText.contains(lbl)) {
                    // Если переводчик удалён, tr вернёт исходный текст (русский)
                    lbl->setText(tr(dynamicLabelsText[lbl].toUtf8()));
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::toggleDock()
{
    if(m_docked) detach();
    else forceDock();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setDocked(bool docked)
{
    m_docked = docked;

    if (flowLayout1) flowLayout1->setWrap(!docked);
    if (flowLayout2) flowLayout2->setWrap(!docked);
    if (flowLayout3) flowLayout3->setWrap(!docked);
    if (flowLayout4) flowLayout4->setWrap(!docked);
    if (flowLayout5) flowLayout5->setWrap(!docked);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::mousePressEvent(QMouseEvent *event)
{
    if (!m_docked && windowFlags().testFlag(Qt::FramelessWindowHint) && event->button() == Qt::LeftButton) {
        QRect r = rect();
        const int margin = 5;

        m_resizeTop    = (event->pos().y() < margin);
        m_resizeBottom = (event->pos().y() > r.height() - margin);
        m_resizeLeft   = (event->pos().x() < margin);
        m_resizeRight  = (event->pos().x() > r.width() - margin);

        if (m_resizeTop || m_resizeBottom || m_resizeLeft || m_resizeRight) {
            m_resizing = true;
            m_dragPosition = event->globalPos();
            event->accept();
            return;
        }
    }
    left_btn_pressed = true;
    QWidget::mousePressEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_docked) {
        // Обновляем курсор и resize
        QRect r = rect();
        const int margin = 5;

        if (m_resizing) {
            QPoint delta = event->globalPos() - m_dragPosition;
            QRect newGeom = geometry();

            if (m_resizeLeft)   newGeom.setLeft(newGeom.left() + delta.x());
            if (m_resizeRight)  newGeom.setRight(newGeom.right() + delta.x());
            if (m_resizeTop)    newGeom.setTop(newGeom.top() + delta.y());
            if (m_resizeBottom) newGeom.setBottom(newGeom.bottom() + delta.y());

            setGeometry(newGeom);
            m_dragPosition = event->globalPos();
            return;
        }

        // курсор
        bool left   = event->pos().x() < margin;
        bool right  = event->pos().x() > r.width() - margin;
        bool top    = event->pos().y() < margin;
        bool bottom = event->pos().y() > r.height() - margin;

        if (top && left) setCursor(Qt::SizeFDiagCursor);
        else if (top && right) setCursor(Qt::SizeBDiagCursor);
        else if (bottom && left) setCursor(Qt::SizeBDiagCursor);
        else if (bottom && right) setCursor(Qt::SizeFDiagCursor);
        else if (top || bottom) setCursor(Qt::SizeVerCursor);
        else if (left || right) setCursor(Qt::SizeHorCursor);
        else setCursor(Qt::ArrowCursor);

        // Проверяем док-позицию
        DockPosition pos = detectDock();

        if (pos != None && left_btn_pressed)
            showDockIndicator(pos);
        else
            hideDockIndicator();
    }
    QWidget::mouseMoveEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_docked && event->button() == Qt::LeftButton) {
        m_resizing = false;
        m_resizeTop = m_resizeBottom = m_resizeLeft = m_resizeRight = false;
    }
    left_btn_pressed = false;
    hideDockIndicator();
    QWidget::mouseReleaseEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::dock()
{
    if (!m_mainWindow->isVisible() || m_mainWindow->isMinimized()) return;

    setParent(m_mainWindow->centralWidget());
    setWindowFlags(Qt::Widget);
    ui.closeBtn->setText("][");
    show();

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_mainWindow->centralWidget()->layout());
    if (!layout) {
        layout = new QVBoxLayout(m_mainWindow->centralWidget());
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
    }
    layout->insertWidget(0, this);

    // фиксируем высоту при доке
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(300);

    setDocked(true);
    m_docked = true;

    // чтобы groupbox не раздувались
    for (auto *box : findChildren<QGroupBox*>()) {
        box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    }
    updateGeometry();
    parentWidget()->updateGeometry();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::forceDock()
{
    setParent(m_mainWindow->centralWidget());
    setWindowFlags(Qt::Widget);
    ui.closeBtn->setText("][");
    show();

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_mainWindow->centralWidget()->layout());
    if (!layout) {
        layout = new QVBoxLayout(m_mainWindow->centralWidget());
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
    }
    layout->insertWidget(0, this);

    // фиксируем высоту при доке
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(300);

    setDocked(true);
    m_docked = true;

    // чтобы groupbox не раздувались
    for (auto *box : findChildren<QGroupBox*>()) {
        box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    }
    updateGeometry();
    parentWidget()->updateGeometry();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::detach()
{
    if (!m_docked) return;

    setParent(nullptr);
    setWindowFlags(Qt::Window
                 | Qt::CustomizeWindowHint
                 | Qt::WindowTitleHint
                 | Qt::WindowSystemMenuHint
                 | Qt::WindowMinimizeButtonHint
                 | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    ui.closeBtn->setText("[ ]");

    // снимаем фиксацию по высоте
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);

    resize(285, 630);  // стартовый размер панели

    // позиционирование относительно курсора
    QPoint desiredPos = QCursor::pos() - QPoint(width()/2, 50); // 50px сверху от курсора

    // границы экрана
    QRect screenRect = QApplication::primaryScreen()->availableGeometry();

    // корректируем, чтобы панель не выходила за экран
    if (desiredPos.x() < screenRect.left()) desiredPos.setX(screenRect.left());
    if (desiredPos.y() < screenRect.top())  desiredPos.setY(screenRect.top());
    if (desiredPos.x() + width()  > screenRect.right()) desiredPos.setX(screenRect.right() - width());
    if (desiredPos.y() + height() > screenRect.bottom()) desiredPos.setY(screenRect.bottom() - height());

    move(desiredPos);

    show();
    m_docked = false;
    setDocked(false); // отключаем wrap
}
//------------------------------------------------------------------------------------------------------------------------------------------

DockPosition QSOPanel::detectDock()
{
    if (!m_mainWindow) return None;

    // Получаем глобальные координаты главного окна
    QRect mainRect = m_mainWindow->geometry();

    // Получаем глобальные координаты панели
    QPoint panelTopLeft = mapToGlobal(QPoint(0,0));
    QRect panelRect(panelTopLeft, size());

    const int snap = 40; // расстояние для "притягивания"

    // Проверяем только верхнюю док-позицию
    bool nearTop = abs(panelRect.top() - mainRect.top()) < snap;
    bool horizontallyAligned = panelRect.right() > mainRect.left() + 20 &&
                               panelRect.left() < mainRect.right() - 20;

    if (nearTop && horizontallyAligned) return Top;
    return None;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::tryDock()
{
    if (!m_mainWindow->isVisible() || m_mainWindow->isMinimized())
        return; // запрещаем докирование если окно свернуто

    if(detectDock() == Top) dock();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::showDockIndicator(DockPosition pos)
{
    if (pos == None) {
        hideDockIndicator();
        return;
    }

    if (!m_rubberBand)
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, m_mainWindow);

    QRect rect;
    if (pos == Top) {
        // Берём область центрального виджета
        QWidget *cw = m_mainWindow->centralWidget();
        if (cw) {
            QRect cwRect = cw->rect();
            // Преобразуем в координаты главного окна
            QPoint topLeft = cw->mapTo(m_mainWindow, cwRect.topLeft());
            rect = QRect(topLeft, QSize(cwRect.width(), height()));
        } else {
            // fallback: вся ширина окна
            rect = QRect(0, 0, m_mainWindow->width(), height());
        }
    }
    m_rubberBand->setGeometry(rect);
    m_rubberBand->show();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::hideDockIndicator()
{
    if(m_rubberBand) m_rubberBand->hide();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::closeEvent(QCloseEvent *event)
{
    forceDock();
    event->ignore();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_docked) {
        for (auto *box : findChildren<QGroupBox*>()) {
            box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        }
        updateGeometry();
        if (parentWidget()) parentWidget()->updateGeometry();
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

// Создание пары QLabel + input
QWidget* QSOPanel::makeInputPair(const QString &labelText, QWidget *inputWidget)
{
    QWidget *pair = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(pair);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(5); // фиксированное расстояние между QLabel и input

    QLabel *label = new QLabel(labelText);
    layout->addWidget(label, 0);          // QLabel не растягивается
    flowLayoutWidgets.append(label);
    dynamicLabelsText[label] = labelText;
    layout->addWidget(inputWidget, 1);    // input растягивается горизонтально
    layout->addStretch(0);                // свободное место после input

    pair->setContentsMargins(0,2,0,2);   // уменьшенные отступы внутри пары
    return pair;
}
//------------------------------------------------------------------------------------------------------------------------------------------

// Создание пары Widget1 + Widget2
QWidget* QSOPanel::makeWidgetPair(QWidget *Widget1, QWidget *Widget2)
{
    QWidget *pair = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(pair);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(5); // фиксированное расстояние между Widget1 и Widget2

    layout->addWidget(Widget1, 0);    // Widget1 не растягивается
    layout->addWidget(Widget2, 1);    // Widget2 растягивается горизонтально
    layout->addStretch(0);            // свободное место после input

    pair->setContentsMargins(0,0,0,0); // уменьшенные отступы внутри пары
    return pair;
}
//------------------------------------------------------------------------------------------------------------------------------------------
// Создание пары QString + Widget1 + Widget2

QWidget* QSOPanel::makeLabelWidgetPair(const QString &labelText, QWidget *Widget1, QWidget *Widget2)
{
    QWidget *pair = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(pair);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(5); // фиксированное расстояние между Widget1 и Widget2

    QLabel *label = new QLabel(labelText);
    layout->addWidget(label, 0);      // QLabel не растягивается
    flowLayoutWidgets.append(label);
    dynamicLabelsText[label] = labelText;
    layout->addWidget(Widget1, 1);    // Widget1 не растягивается
    layout->addWidget(Widget2, 0);    // Widget2 растягивается горизонтально
    layout->addStretch(0);            // свободное место после input

    pair->setContentsMargins(0,0,0,0); // уменьшенные отступы внутри пары
    return pair;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::UpdateFormDateTime()
{
    QDateTime DateTimeNow = QDateTime::currentDateTimeUtc().toUTC();
    DateEdit->setDate(DateTimeNow.date());
    TimeEdit->setTime(DateTimeNow.time());
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_SaveQsoButton_clicked()
{
    emit saveQSO();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_RefreshButton_clicked()
{
    emit updateDB();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_ClearQsoButton_clicked()
{
    clearQSO();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::minimizePanel()
{
    if (m_docked) {
        this->setVisible(false); // спрятать панель
    } else {
        this->showMinimized();   // если откреплено — сворачиваем окно
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::CallsignToUppercase(const QString &arg)
{
    int cursorPos = CallInput->cursorPosition();
    QString callsign = arg.toUpper();
    CallInput->setText(callsign);
    CallInput->setStyleSheet("font-weight: bold");
    CallInput->setCursorPosition(cursorPos);
    CallTypeTimer->start();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getCallsign()
{
    if (!CallInput) return QString();
    return CallInput->text().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getFrequence()
{
    return FreqInput->text();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getBand()
{
    return BandCombo->currentText();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getMode()
{
    if (!ModeCombo) return QString();
    return ModeCombo->currentText().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getRSTR()
{
    if (!RstrInput) return QString();
    return RstrInput->text();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getRSTS()
{
    if (!RstsInput) return QString();
    return RstsInput->text();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getName()
{
    if (!NameInput) return QString();
    return NameInput->text().trimmed();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getQTH()
{
    if (!QTHInput) return QString();
    return QTHInput->text().trimmed();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getQTHLocator()
{
    if (!GridSquareInput) return QString();
    return GridSquareInput->text().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getRDA()
{
    if (!CNTYInput) return QString();
    return CNTYInput->text().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getComment()
{
    if (!CommentInput) return QString();
    return CommentInput->text();
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setFrequence(long freq)
{
    if (!FreqInput) return false;
    if(freq > 0 && freq < 250000000000) {
        double mhz = freq / 1e6;
        FreqInput->setText(QString::number(mhz, 'f', 6));
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setFrequenceText(const QString &freq)
{
    FreqInput->setText(freq);
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setBand(const QString &band)
{
    QString b = band.trimmed();

    if (b.isEmpty()) return false;
    if (!BandCombo) return false;

    // Пытаемся найти индекс нужного бэнда в комбобоксе
    int idx = BandCombo->findText(b, Qt::MatchFixedString | Qt::MatchCaseSensitive);
    if (idx == -1) return false;

    BandCombo->setCurrentIndex(idx);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setMode(const QString &mode)
{
    QString m = mode.trimmed();
    if (m.isEmpty()) return false;

    if (!ModeCombo) return false;

    int idx = ModeCombo->findText(m, Qt::MatchFixedString | Qt::MatchCaseSensitive);
    if (idx == -1) return false;

    ModeCombo->setCurrentIndex(idx);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setCallsign(const QString &call)
{
    QString c = call.trimmed();
    if (c.isEmpty()) return false;

    if (!CallInput) return false;

    CallInput->setText(c);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------


bool QSOPanel::setName(const QString &name)
{
    QString n = name.trimmed();
    if (n.isEmpty()) return false;

    if (!NameInput) return false;

    NameInput->setText(n);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setQTH(const QString &qth)
{
    QString q = qth.trimmed();
    if (q.isEmpty()) return false;

    if (!QTHInput) return false;

    QTHInput->setText(q);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setRSTR(const QString &rstr)
{
    QString r = rstr.trimmed();
    if (r.isEmpty()) return false;

    if (!RstrInput) return false;

    RstrInput->setText(r);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setRSTS(const QString &rsts)
{
    QString r = rsts.trimmed();
    if (r.isEmpty()) return false;

    if (!RstsInput) return false;

    RstsInput->setText(r);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setQTHLocator(const QString &loc)
{
    QString l = loc.trimmed();
    if (l.isEmpty()) return false;

    if (!GridSquareInput) return false;

    GridSquareInput->setText(l);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setRDA(const QString &rda)
{
    QString r = rda.trimmed();
    if (r.isEmpty()) return false;

    if (!CNTYInput) return false;

    CNTYInput->setText(r);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setStationQTHLocator(const QString &loc)
{
    QString l = loc.trimmed();
    if (l.isEmpty()) return false;

    if (!QTHLocEdit) return false;

    QTHLocEdit->setText(l);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setStationRDA(const QString &rda)
{
    QString r = rda.trimmed();
    if (r.isEmpty()) return false;

    if (!RDAEdit) return false;

    RDAEdit->setText(r);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setComment(const QString &comment)
{
    QString c = comment.trimmed();
    if (c.isEmpty()) return false;

    if (!CommentInput) return false;

    CommentInput->setText(c);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getStationCallsign()
{
    if (!stationCallsign) return QString();
    return stationCallsign->currentText().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getStationOperator()
{
    if (!operatorCallsign) return QString();
    return operatorCallsign->currentText().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getStationQTHLocator()
{
    if (!QTHLocEdit) return QString();
    return QTHLocEdit->text().trimmed().toUpper();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QString QSOPanel::getStationRDA()
{
    if (!RDAEdit) return QString();
    return RDAEdit->text().trimmed().toUpper();;
}
//------------------------------------------------------------------------------------------------------------------------------------------

QDate QSOPanel::getDate()
{
    if (!DateEdit) return QDate(); // вернёт пустую дату, если поле отсутствует
    return DateEdit->date();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QTime QSOPanel::getTime()
{
    if (!TimeEdit) return QTime(); // вернёт пустое время, если поле отсутствует
    return TimeEdit->time();
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool QSOPanel::setDate(const QDate &date)
{
    if (!date.isValid()) return false;
    DateEdit->setDate(date);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------
bool QSOPanel::setTime(const QTime &time)
{
    if (!time.isValid()) return false;
    TimeEdit->setTime(time);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::clearQSO()
{
    CallInput->clear();
    NameInput->clear();
    QTHInput->clear();
    GridSquareInput->clear();
    CNTYInput->clear();
    CommentInput->clear();
    RstrInput->clear();
    RstsInput->clear();
    QSOSUserIcon->setVisible(false);
    QSOSUserLabel->setVisible(false);
    UserSRRIcon->setVisible(false);
    UserSRRLabel->setVisible(false);
    countryFlag->setVisible(false);
    countryName->setVisible(false);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setQSOSUserVisible(bool visible)
{
    QSOSUserIcon->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setUserSRRVisible(bool visible)
{
    UserSRRIcon->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setQSOSUserLabelVisible(bool visible)
{
    QSOSUserLabel->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setUserSRRLabelVisible(bool visible)
{
    UserSRRLabel->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setQSOSUserLabelText(QString text)
{
    QSOSUserLabel->setText(text);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setUserSRRLabelText(QString text)
{
    UserSRRLabel->setText(text);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setQSOSUserLabelStyle(QString style)
{
    QSOSUserLabel->setStyleSheet(style);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setUserSRRLabelStyle(QString style)
{
    UserSRRLabel->setStyleSheet(style);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setQSOSUserPixmap(const QPixmap &pix)
{
    QSOSUserIcon->setPixmap(pix);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setUserSRRPixmap(const QPixmap &pix)
{
    UserSRRIcon->setPixmap(pix);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::addBandItems(const QString &item)
{
    BandCombo->addItem(item);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::addModeItems(const QString &item)
{
    ModeCombo->addItem(item);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setCallsignCompleter(QCompleter *completer)
{
    CallInput->setCompleter(completer);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::addStationCallsignItems(const QString &name, const QList<QVariant> &item)
{
    stationCallsign->addItem(name, QVariant::fromValue(item));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::addOperatorCallsignItems(const QString &name, const QList<QVariant> &item)
{
    operatorCallsign->addItem(name, QVariant::fromValue(item));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::clearStationCallsignItems()
{
    stationCallsign->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::clearOperatorCallsignItems()
{
    operatorCallsign->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

QList<QVariant> QSOPanel::getStationCallsignItems(const QString &call)
{
    int index = stationCallsign->findText(call);
    if (index >= 0) {
        return stationCallsign->itemData(index).value<QList<QVariant>>();
    }
    return {};
}

//------------------------------------------------------------------------------------------------------------------------------------------

QList<QVariant> QSOPanel::getStationOperatorItems(const QString &call)
{
    int index = operatorCallsign->findText(call);
    if (index >= 0) {
        return operatorCallsign->itemData(index).value<QList<QVariant>>();
    }
    return {};
}

//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_StationCallsignCurrentIndexChanged(int index)
{
    emit stationCallsignIndexChanged(index);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_BandComboCurrentIndexChanged(int index)
{
    CAT->setBand(index);
    emit bandIndexChanged(index);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_OperatorCallsignCurrentIndexChanged(int index)
{
    emit operatorCallsignIndexChanged(index);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_BandComboCurrentTextChanged(const QString &arg1)
{
     emit bandTextChanged(arg1);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_ModeComboCurrentTextChanged(const QString &arg1)
{    
    CAT->setMode(ModeCombo->findText(arg1));
    emit modeTextChanged(arg1);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_FreqInputTextChanged(const QString &arg1)
{
    double freqCat = static_cast<long>(FreqInput->text().toDouble() * 1000000);

    CAT->setFreq(freqCat);
    emit freqTextChanged(arg1);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_FreqInputEditingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_RstrInputEditingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_RstsInputEditingFinished()
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_GridSquareInputTextChanged(const QString &arg1)
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::on_CNTYInputTextChanged(const QString &arg1)
{
    SaveFormData();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::ClearCallbookFields()
{
   NameInput->clear();
   QTHInput->clear();
   GridSquareInput->clear();
   CNTYInput->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::SaveFormData()
{
    settings->lastLocator = GridSquareInput->text().trimmed().toUpper();
    settings->lastRDA = CNTYInput->text().trimmed().toUpper();
    settings->lastFrequence = FreqInput->text();
    settings->lastRST_RCVD = RstrInput->text();
    settings->lastRST_SENT = RstsInput->text();
    settings->saveForm();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::SaveCallsignState()
{
    settings->lastCallsign = stationCallsign->currentIndex();
    settings->lastOperator = operatorCallsign->currentIndex();
    settings->saveForm();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setStationCallsignCurrentIndex(int idx)
{
    stationCallsign->setCurrentIndex(idx);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setStationOperatorCurrentIndex(int idx)
{
    operatorCallsign->setCurrentIndex(idx);
}
//------------------------------------------------------------------------------------------------------------------------------------------

int QSOPanel::getBandCurrentIndex()
{
    return BandCombo->currentIndex();
}
//------------------------------------------------------------------------------------------------------------------------------------------

int QSOPanel::getModeTextIndex()
{
    return ModeCombo->findText(settings->lastMode);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setFlag(const QString &countryCode)
{
    // Загружаем флаг, если существует
    QString iconPath = QString(":resources/flags/%1.png").arg(countryCode);
    if (QFile::exists(iconPath)) {
        QPixmap pix(iconPath);
        QPixmap scaled = pix.scaled(30, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        countryFlag->setPixmap(QPixmap(scaled));
        countryFlag->setVisible(true);
    } else {
        countryFlag->setVisible(false);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setCountry(const QString &country)
{
    if(country != "")
    {
        countryName->setText(country);
        countryName->setVisible(true);
    } else {
        countryName->setText("");
        countryName->setVisible(false);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setFlagVisible(bool visible)
{
    countryFlag->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void QSOPanel::setCountryVisible(bool visible)
{
    countryName->setVisible(visible);
}
//------------------------------------------------------------------------------------------------------------------------------------------













