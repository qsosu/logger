/**********************************************************************************************************
Description :  Implementation of the ChatController class, which sends, receives, and displays messages
            :  over the network from the QSO.SU server.
Version     :  1.0.0
Date        :  10.09.2025
Author      :  R9JAU
Comments    :
***********************************************************************************************************/

#include "chatcontroller.h"
#include "ui_chatcontroller.h"
#include "telnetclient.h"

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QDebug>
#include <QLabel>


ChatController::ChatController(QSqlDatabase db, HttpApi *api, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatController)
{
    this->db = db;
    this->api = api;
    ui->setupUi(this);

    currentUser = ""; // пока пустой, будет выбран из ComboBox

    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    flags |= Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;
    this->setWindowFlags(flags);

    tabs = new QTabWidget(this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    // загрузим свои позывные из БД
    loadCallsigns();

    // сигналы
    connect(api, &HttpApi::chatsLoaded, this, &ChatController::onChatsLoaded);
    connect(api, &HttpApi::chatWithMessagesLoaded, this, &ChatController::onChatWithMessagesLoaded);
    connect(api, &HttpApi::messageSent, this, [this](int chatId, const Message &msg) {
        if (!chatViews.contains(chatId)) return;
        ChatTab &tab = chatViews[chatId];
        displayMessage(tab, msg);
    });
    api->getChats();
}
//------------------------------------------------------------------------------------------------------------------------------------------

ChatController::~ChatController()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::showEvent(QShowEvent *event)
{

}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::onChatsLoaded(const QList<Chat> &chats)
{
    for (const auto &chatRef : chats)
    {
        Chat chat = chatRef;

        QPointer<QWidget> tab = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(tab);

        QPointer<QListWidget> list = new QListWidget;

        list->setSpacing(4);
        list->setStyleSheet("QListWidget { border: none; }");
        list->setSelectionMode(QAbstractItemView::NoSelection);
        list->setFocusPolicy(Qt::NoFocus);
        list->viewport()->installEventFilter(this);

        // обработчик двойного клика
        connect(list, &QListWidget::itemDoubleClicked, this, [this, chat](QListWidgetItem *item) {
            if (!chatViews.contains(chat.id)) return;
            ChatTab &tab = chatViews[chat.id];

            QString sender = item->data(Qt::UserRole).toString();
            if (!sender.isEmpty() && tab.input) {
                // только если это не наш позывной
                if (!ownCallsigns.contains(sender)) {
                    tab.input->setText(sender + ": ");
                    tab.input->setFocus();
                }
            }
        });
        layout->addWidget(list);

        // Таймер для временных чатов
        QPointer<QLabel> infoLabel = nullptr;
        QPointer<QTimer> countdownTimer = nullptr;

        if (chat.isTemporary) {
            QDateTime expires = QDateTime::fromString(chat.expiresAt, Qt::ISODate);
            expires.setTimeSpec(Qt::UTC);

            infoLabel = new QLabel;
            infoLabel->setAlignment(Qt::AlignCenter);
            infoLabel->setStyleSheet(
                "font-size: 8pt;"
                "color: rgb(255, 102, 102);"
                "background-color: rgb(255, 236, 236);"
            );
            infoLabel->setMinimumHeight(24);
            layout->insertWidget(0, infoLabel); // сверху

            countdownTimer = new QTimer(this);
            countdownTimer->setInterval(1000); // обновление каждую секунду

            auto updateInfo = [infoLabel, expires, countdownTimer]() {
                if (!infoLabel) return;

                qint64 secsRemaining = QDateTime::currentDateTimeUtc().secsTo(expires);
                if (secsRemaining <= 0) {
                    infoLabel->setText(QObject::tr("Чат закрыт"));
                    if (countdownTimer) countdownTimer->stop();
                    return;
                }

                if (secsRemaining > 24 * 3600) {
                    infoLabel->setText(QObject::tr("Чат закроется %1 в %2")
                                           .arg(expires.toLocalTime().toString("dd.MM.yyyy"))
                                           .arg(expires.toLocalTime().toString("HH:mm")));
                } else if (secsRemaining > 12 * 3600) {
                    int hours = secsRemaining / 3600;
                    int minutes = (secsRemaining % 3600) / 60;
                    infoLabel->setText(QObject::tr("Чат закроется через %1:%2.")
                                           .arg(hours)
                                           .arg(minutes, 2, 10, QLatin1Char('0')));
                } else {
                    int hours = secsRemaining / 3600;
                    int minutes = (secsRemaining % 3600) / 60;
                    int secs = secsRemaining % 60;
                    infoLabel->setText(QObject::tr("Чат закроется через %1:%2:%3.")
                                           .arg(hours)
                                           .arg(minutes, 2, 10, QLatin1Char('0'))
                                           .arg(secs, 2, 10, QLatin1Char('0')));
                }
            };
            connect(countdownTimer, &QTimer::timeout, this, updateInfo);
            countdownTimer->start();
            updateInfo(); // сразу начальное значение
        }

        QPointer<QWidget> inputContainer = new QWidget;
        QVBoxLayout *inputLayout = new QVBoxLayout(inputContainer);
        inputLayout->setContentsMargins(0,0,0,0);
        inputLayout->setSpacing(2);

        QPointer<QLineEdit> input = new QLineEdit;
        input->setMaxLength(255);
        input->setPlaceholderText(tr("Введите сообщение"));

        QPointer<QPushButton> sendBtn = new QPushButton(tr("Отправить"));
        QPointer<QComboBox> callCombo = new QComboBox;

        QPointer<QLabel> charsLeftLabel = new QLabel;
        charsLeftLabel->setStyleSheet("font-size: 9px;");
        charsLeftLabel->setAlignment(Qt::AlignLeft);

        inputLayout->addWidget(input);
        inputLayout->addWidget(charsLeftLabel);

        QHBoxLayout *bottomLayout = new QHBoxLayout;
        bottomLayout->addWidget(callCombo);
        bottomLayout->addWidget(inputContainer);
        bottomLayout->addWidget(sendBtn);
        layout->addLayout(bottomLayout);

        auto updateCharsLeft = [input, charsLeftLabel]() {
            if (!input || !charsLeftLabel) return;
            int remaining = input->maxLength() - input->text().length();
            charsLeftLabel->setVisible(remaining <= 50);
            if (remaining <= 50)
                charsLeftLabel->setText(QObject::tr("Осталось: ") + QString::number(remaining) + QObject::tr(" символов"));
        };
        updateCharsLeft();
        if (input) connect(input, &QLineEdit::textChanged, updateCharsLeft);

        tabs->addTab(tab, chat.name);

        ChatTab chatTab;
        chatTab.messageList = list;
        chatTab.input = input;
        chatTab.sendButton = sendBtn;
        chatTab.callCombo = callCombo;
        chatTab.timerLabel = infoLabel;
        chatTab.countdownTimer = countdownTimer;
        chatViews[chat.id] = chatTab;

        fillCallsigns(callCombo);
        if (callCombo && callCombo->count() > 0) {
            callCombo->setCurrentIndex(0);
            currentUser = callCombo->currentText();
        }

        if (sendBtn) connect(sendBtn, &QPushButton::clicked, this, [this, chat]() {
            sendMessage(chat.id);
        });
        if (input) connect(input, &QLineEdit::returnPressed, this, [this, chat]() {
            sendMessage(chat.id);
        });
        if (callCombo) connect(callCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, chat]() {
            if (chatViews.contains(chat.id)) {
                ChatTab &tab = chatViews[chat.id];
                if (tab.callCombo && tab.callCombo->currentIndex() >= 0)
                    currentUser = tab.callCombo->currentText();
            }
        });
        // Получаем сообщения
        api->getChatMessages(chat.id);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::onMessageReceived(const Message &msg)
{
    if (!chatViews.contains(msg.chatId)) return;
    ChatTab &tab = chatViews[msg.chatId];

    // Добавляем сообщение в список истории
    tab.messages.append(msg);

    // Добавляем только новое сообщение в QListWidget
    displayMessage(tab, msg);

    // Скроллим к последнему элементу
    if (tab.messageList->count() > 0)
        tab.messageList->scrollToItem(tab.messageList->item(tab.messageList->count() - 1), QAbstractItemView::PositionAtBottom);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::onChatWithMessagesLoaded(const Chat &chat, const QList<Message> &messages)
{
    if (!chatViews.contains(chat.id)) return;
    ChatTab &tab = chatViews[chat.id];

    tab.messageList->clear();

    // Сортировка сообщений по дате: старые сверху, новые снизу
    QList<Message> sortedMessages = messages;
    std::sort(sortedMessages.begin(), sortedMessages.end(), [](const Message &a, const Message &b) {
        return QDateTime::fromString(a.sentAt, Qt::ISODate) < QDateTime::fromString(b.sentAt, Qt::ISODate);
    });

    // Добавляем сообщения по порядку в конец
    for (const Message &msg : sortedMessages) {
        displayMessage(tab, msg);
    }

    // Автоскролл вниз после добавления всех сообщений
    if (tab.messageList->count() > 0)
        tab.messageList->scrollToItem(
        tab.messageList->item(tab.messageList->count() - 1),
        QAbstractItemView::PositionAtBottom);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::sendMessage(int chatId)
{
    if (!chatViews.contains(chatId)) return;
    ChatTab &tab = chatViews[chatId];

    QString text = tab.input->text().trimmed();
    if (text.isEmpty()) return;

    int callsignId = tab.callCombo->currentData().toInt();
    currentUser = tab.callCombo->currentText();

    api->sendMessage(chatId, callsignId, text);
    qInfo() << "Message transmited!";

    tab.input->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::displayMessage(ChatTab &tab, const Message &msg)
{
    if (msg.text.trimmed().isEmpty()) return;

    const QDateTime dt = QDateTime::fromString(msg.sentAt, Qt::ISODate);
    const QString formattedTime = dt.toString("dd.MM.yyyy HH:mm");

    bool isSelf = ownCallsigns.contains(msg.sender);

    const QString bgColor = msg.backgroundColor.isEmpty()
            ? (isSelf ? "#D0F0C0" : "#E0E0E0")
            : msg.backgroundColor;
    const QString textColor = msg.textColor.isEmpty() ? "black" : msg.textColor;

    QWidget *bubbleWidget = new QWidget;
    bubbleWidget->setProperty("isBubble", true);

    QVBoxLayout *vLayout = new QVBoxLayout(bubbleWidget);
    vLayout->setContentsMargins(4, 2, 4, 2);
    vLayout->setSpacing(0);

    QLabel *textBlock = new QLabel(
        "<b>" + msg.sender.toHtmlEscaped() + "</b><br>" +
        msg.text.toHtmlEscaped().replace("\n", "<br>")
    );
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setStyleSheet("color:" + textColor + "; margin-bottom:0px;");
    textBlock->setWordWrap(true);
    textBlock->setAlignment(isSelf ? Qt::AlignRight : Qt::AlignLeft);
    textBlock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QLabel *timeLabel = new QLabel(formattedTime);
    timeLabel->setStyleSheet(QString("color:%1; font-size:9px;").arg(textColor));
    timeLabel->setAlignment(isSelf ? Qt::AlignRight : Qt::AlignLeft);
    timeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    vLayout->addWidget(textBlock);
    vLayout->addSpacing(6);  // фикс. расстояние между текстом и временем
    vLayout->addWidget(timeLabel);

    bubbleWidget->setStyleSheet(QString("QWidget { background-color:%1; border-radius:10px; padding:4px; }").arg(bgColor));
    bubbleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    // сразу подгоняем ширину под окно
    int bubbleMaxWidth = tab.messageList->viewport()->width() * 0.48;
    bubbleWidget->setMinimumWidth(bubbleMaxWidth);
    bubbleWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QListWidgetItem *item = new QListWidgetItem(tab.messageList);
    item->setData(Qt::UserRole, msg.sender);
    // сохраняем пузырь для быстрого ресайза
    item->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget*>(bubbleWidget));

    QWidget *rowWidget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(rowWidget);
    hLayout->setContentsMargins(8, 2, 8, 2);
    hLayout->setSpacing(2);

    if (isSelf) {
        hLayout->addStretch();
        hLayout->addWidget(bubbleWidget, 0, Qt::AlignRight);
    } else {
        hLayout->addWidget(bubbleWidget, 0, Qt::AlignLeft);
        hLayout->addStretch();
    }

    tab.messageList->setItemWidget(item, rowWidget);
    item->setSizeHint(rowWidget->sizeHint());

    tab.messageList->addItem(item);
    tab.messageList->scrollToBottom();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::loadCallsigns()
{
    ownCallsigns.clear();
    QSqlQuery query;
    if (query.exec("SELECT name FROM callsigns ORDER BY name")) {
        while (query.next()) {
            ownCallsigns.insert(query.value(0).toString());
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::fillCallsigns(QComboBox *combo)
{
    combo->clear();
    QSqlQuery query(db);
    if (!query.exec("SELECT qsosu_id, name FROM callsigns ORDER BY name")) {
        qDebug() << "DB error:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        combo->addItem(name, id);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------

bool ChatController::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        for (auto it = chatViews.begin(); it != chatViews.end(); ++it)
        {
            ChatTab &tab = it.value();
            if (watched != tab.messageList->viewport()) continue;
            QListWidget *list = tab.messageList;
            // новая макс ширина пузыря
            int bubbleMaxWidth = list->viewport()->width() * 0.48;
            // отключаем перерасчёт лейаута на время массовых операций (ускоряет в 10–20 раз)
            list->setUpdatesEnabled(false);

            for (int i = 0; i < list->count(); ++i)
            {
                QListWidgetItem *item = list->item(i);
                QWidget *bubble = item->data(Qt::UserRole + 1).value<QWidget*>();
                if (!bubble) continue;
                // применяем новую ширину
                bubble->setMinimumWidth(bubbleMaxWidth);
                // пересчитываем размер строки
                QWidget *rowWidget = list->itemWidget(item);
                if (rowWidget) item->setSizeHint(rowWidget->sizeHint());
            }
            // один раз пересчитать layout
            list->doItemsLayout();
            // включаем обновления
            list->setUpdatesEnabled(true);
        }
    }
    return QDialog::eventFilter(watched, event);
}
//------------------------------------------------------------------------------------------------------------------------------------------
