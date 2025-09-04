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

void ChatController::onChatsLoaded(const QList<Chat> &chats)
{
    for (const auto &chat : chats) {
        // используем QPointer для безопасности
        QPointer<QWidget> tab = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(tab);

        QPointer<QListWidget> list = new QListWidget;
        list->setSpacing(4);
        list->setStyleSheet("QListWidget { border: none; }");
        layout->addWidget(list);

        QPointer<QWidget> inputContainer = new QWidget;
        QVBoxLayout *inputLayout = new QVBoxLayout(inputContainer);
        inputLayout->setContentsMargins(0,0,0,0);
        inputLayout->setSpacing(2);

        QPointer<QLineEdit> input = new QLineEdit;
        input->setMaxLength(255);
        input->setPlaceholderText("Введите сообщение");

        QPointer<QPushButton> sendBtn = new QPushButton("Отправить");
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
            if (remaining <= 50) charsLeftLabel->setText("Осталось: " + QString::number(remaining) + " символов");
        };
        updateCharsLeft();
        if (input) connect(input, &QLineEdit::textChanged, updateCharsLeft);

        tabs->addTab(tab, chat.name);

        // Сохраняем виджеты в структуре ChatTab
        ChatTab chatTab;
        chatTab.messageList = list;
        chatTab.input = input;
        chatTab.sendButton = sendBtn;
        chatTab.callCombo = callCombo;
        chatViews[chat.id] = chatTab;

        fillCallsigns(callCombo);
        if (callCombo && callCombo->count() > 0) {
            callCombo->setCurrentIndex(0);
            currentUser = callCombo->currentText();
        }

        // Подключаем сигналы через QPointer
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

        // Получаем сообщения после создания вкладки
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
        tab.messageList->scrollToItem(
                    tab.messageList->item(tab.messageList->count() - 1),
                    QAbstractItemView::PositionAtBottom
                    );
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
    qDebug() << "Message transmited";

    tab.input->clear();
}
//------------------------------------------------------------------------------------------------------------------------------------------

void ChatController::displayMessage(ChatTab &tab, const Message &msg)
{
    if (msg.text.trimmed().isEmpty()) return; // игнор пустых сообщений

    const QDateTime dt = QDateTime::fromString(msg.sentAt, Qt::ISODate);
    const QString formattedTime = dt.toString("dd.MM.yyyy HH:mm");

    bool isSelf = ownCallsigns.contains(msg.sender); // свои сообщения по списку позывных

    const QString bgColor   = msg.backgroundColor.isEmpty()
            ? (isSelf ? "#D0F0C0" : "#E0E0E0")
            : msg.backgroundColor;
    const QString textColor = msg.textColor.isEmpty() ? "black" : msg.textColor;

    // Виджет "пузырь"
    QWidget *bubbleWidget = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout(bubbleWidget);
    vLayout->setContentsMargins(4,2,4,2);
    vLayout->setSpacing(0);

    // Sender жирным
    QLabel *senderLabel = new QLabel("<b>" + msg.sender.toHtmlEscaped() + "</b>");
    senderLabel->setStyleSheet("color:" + textColor);
    senderLabel->setWordWrap(true);
    senderLabel->setAlignment(isSelf ? Qt::AlignRight : Qt::AlignLeft);
    senderLabel->setContentsMargins(0,0,0,0);

    // Текст сообщения
    QLabel *textLabel = new QLabel(msg.text.toHtmlEscaped().replace("\n","<br>"));
    textLabel->setStyleSheet("color:" + textColor);
    textLabel->setWordWrap(true);
    textLabel->setAlignment(isSelf ? Qt::AlignRight : Qt::AlignLeft);
    textLabel->setContentsMargins(0,0,0,0);

    // Время
    QLabel *timeLabel = new QLabel(formattedTime);
    timeLabel->setStyleSheet(QString("color:%1; font-size:9px;").arg(textColor));
    timeLabel->setAlignment(isSelf ? Qt::AlignRight : Qt::AlignLeft);
    timeLabel->setContentsMargins(0,0,0,0);

    vLayout->addWidget(senderLabel);
    vLayout->addWidget(textLabel);
    vLayout->addWidget(timeLabel);

    bubbleWidget->setStyleSheet(QString("QWidget { background-color:%1; border-radius:10px; padding:4px; }").arg(bgColor));
    bubbleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    bubbleWidget->setMinimumWidth(150);   // минимальная ширина пузыря
    bubbleWidget->setMaximumWidth(300);   // максимальная ширина пузыря

    // Элемент списка
    QListWidgetItem *item = new QListWidgetItem(tab.messageList);

    // Горизонтальный контейнер для выравнивания
    QWidget *rowWidget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(rowWidget);
    hLayout->setContentsMargins(8,2,8,2);
    hLayout->setSpacing(2);

    if (isSelf) {
        hLayout->addStretch();
        hLayout->addWidget(bubbleWidget,0,Qt::AlignRight);
    } else {
        hLayout->addWidget(bubbleWidget,0,Qt::AlignLeft);
        hLayout->addStretch();
    }

    tab.messageList->setItemWidget(item, rowWidget);
    item->setSizeHint(rowWidget->sizeHint());

    // Вставляем элемент в конец списка, чтобы новые сообщения были снизу
    tab.messageList->addItem(item);
    tab.messageList->scrollToBottom();
    tab.messageList->updateGeometry();
    tab.messageList->viewport()->update();
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
