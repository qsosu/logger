#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QDialog>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QWidget>
#include <QDebug>
#include "httpapi.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

struct ChatTab {
    QListWidget *messageList;
    QLineEdit *input;
    QPushButton *sendButton;
    QComboBox *callCombo;
    QList<Message> messages; // для сортировки
};

namespace Ui {
class ChatController;
}

class ChatController : public QDialog
{
    Q_OBJECT

public:
    explicit ChatController(QSqlDatabase db, HttpApi *api, QWidget *parent = nullptr);
    ~ChatController();
    void displayMessage(ChatTab &tab, const Message &msg);
    void fillCallsigns(QComboBox *combo);
    void loadCallsigns();

public slots:
    void onMessageReceived(const Message &msg);

private slots:
    void onChatsLoaded(const QList<Chat> &chats);
    void onChatWithMessagesLoaded(const Chat &chat, const QList<Message> &messages);
    void sendMessage(int chatId);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::ChatController *ui;
    QTabWidget *tabs;
    HttpApi *api;
    QSqlDatabase db;
    QString currentUser;
    QMap<int, ChatTab> chatViews;
    QSet<QString> ownCallsigns;
};

#endif // CHATCONTROLLER_H
