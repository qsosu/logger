#ifndef ADDCALLSIGN_H
#define ADDCALLSIGN_H

#include <QDialog>

namespace Ui {
class Addcallsign;
}

class Addcallsign : public QDialog
{
    Q_OBJECT

public:
    explicit Addcallsign(QWidget *parent = nullptr);
    ~Addcallsign();

    QString add_Callsign;
    int add_CallsignType;
    int add_validity_start;
    int add_validity_stop;
    QString add_location;
    QString add_rda;
    int add_ituz;
    int add_cqz;

private slots:
    void on_addCallsignEdit_textEdited(const QString &arg1);
    void on_addCallsignType_currentIndexChanged(int index);
    void on_OkCallsignBtn_clicked();
    void on_addRDA_textEdited(const QString &arg1);
    void on_addLOC_textEdited(const QString &arg1);
    void on_CnlCallsignBtn_clicked();

signals:
    void addCallsign();

private:
    Ui::Addcallsign *ui;

};

#endif // ADDCALLSIGN_H
