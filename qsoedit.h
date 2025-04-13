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
    explicit Qsoedit(QSqlDatabase db, Settings *settings, QWidget *parent = nullptr);
    ~Qsoedit();
    void ShowQSOParams(QVariantList data);
    void noneImage();
    void getCallsigns();
    QString getHashByID(int db_id);
    QString getCallsignName(int id);
    int getCallsignID(QString callsign);
    int getLocalCallsignID(QString callsign);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void setVisible(bool set) override;
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void on_QRZUpdateButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void loadImage(QPixmap pix);
    void onResizeFinished();
    void setUserData();
    void updateQSOData(QString hash);
    void errorUpdateQSOData(QString error);
    //void onQSOConfirmed();

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
    QStringList userData;

    bool load_flag;
    QTimer *resizeTimer; // Таймер для отслеживания окончания изменения размера
    int qsosu_id;
    int type;

signals:
    void db_updated();

};

#endif // QSOEDIT_H
