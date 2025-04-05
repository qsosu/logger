#ifndef QSOEDIT_H
#define QSOEDIT_H

#include <QDialog>
#include "qrzrucallbook.h"
#include "settings.h"
#include "httpapi.h"
#include <QtWidgets>
#include <QTimer>

namespace Ui {
class Qsoedit;
}

class Qsoedit : public QDialog
{
    Q_OBJECT

public:
    explicit Qsoedit(QSqlDatabase db, QWidget *parent = nullptr);
    ~Qsoedit();
    void ShowQSOParams(QVariantList data);
    void noneImage();


protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void setVisible(bool set) override;

private slots:
    void on_QRZUpdateButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void loadImage(QPixmap pix);
    void onResizeFinished();
    void setUserData();

private:
    Ui::Qsoedit *ui;
    QSqlDatabase db;
    QTimer *CallTypeTimer;
    QrzruCallbook *qrz;
    Settings *settings;
    HttpApi *api;
    int dbid;
    QString image;
    QGraphicsPixmapItem *pixmap_item;
    QGraphicsScene *scene;
    QGraphicsTextItem *textItem;
    void closeEvent(QCloseEvent *event);
    bool load_flag;
    QTimer *resizeTimer; // Таймер для отслеживания окончания изменения размера

signals:
    void db_updated();

};

#endif // QSOEDIT_H
