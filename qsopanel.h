#ifndef QSOPANEL_H
#define QSOPANEL_H

#include <QWidget>
#include <QMainWindow>
#include "ui_qsopanel.h"
#include <QMouseEvent>
#include <QFrame>
#include <QRubberBand>
#include <QCompleter>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QDateTimeEdit>
#include <QCheckBox>

#include "settings.h"
#include "flowlayout.h"

enum DockPosition { None, Top };

class QSOPanel : public QWidget
{
    Q_OBJECT
public:
    explicit QSOPanel(QMainWindow *mainWindow, Settings *settings, cat_Interface *cat, QWidget *parent = nullptr);
    void dock();
    void forceDock();
    void detach();
    void setDocked(bool docked);
    QString getStationCallsign();
    QString getStationOperator();
    QString getStationQTHLocator();
    QString getStationRDA();
    QString getCallsign();
    QString getName();
    QString getFrequence();
    QString getBand();
    int getBandCurrentIndex();
    int getModeTextIndex();
    QString getMode();
    QDate getDate();
    QTime getTime();
    QString getRSTR();
    QString getRSTS();
    QString getQTH();
    QString getQTHLocator();
    QString getRDA();
    QString getComment();

    bool setCallsign(const QString &call);
    void setFlag(const QString &countryCode);
    void setCountry(const QString &country);
    bool setName(const QString &name);
    bool setQTH(const QString &qth);
    bool setRSTR(const QString &rstr);
    bool setRSTS(const QString &rsts);
    bool setQTHLocator(const QString &loc);
    bool setRDA(const QString &rda);
    bool setStationQTHLocator(const QString &loc);
    bool setStationRDA(const QString &rda);
    bool setComment(const QString &comment);
    bool setFrequence(long freq);
    void setFrequenceText(const QString &freq);
    bool setBand(const QString &band);
    bool setMode(const QString &mode);
    bool setDate(const QDate &date);
    bool setTime(const QTime &time);
    void clearQSO();
    void setFlagVisible(bool visible);
    void setCountryVisible(bool visible);
    void setQSOSUserVisible(bool visible);
    void setUserSRRVisible(bool visible);
    void setQSOSUserLabelVisible(bool visible);
    void setUserSRRLabelVisible(bool visible);
    void setQSOSUserPixmap(const QPixmap &pix);
    void setUserSRRPixmap(const QPixmap &pix);
    void setQSOSUserLabelText(QString text);
    void setUserSRRLabelText(QString text);
    void setQSOSUserLabelStyle(QString style);
    void setUserSRRLabelStyle(QString style);
    void addStationCallsignItems(const QString &name, const QList<QVariant> &item);
    void addOperatorCallsignItems(const QString &name, const QList<QVariant> &item);
    void addBandItems(const QString &item);
    void addModeItems(const QString &item);
    void setCallsignCompleter(QCompleter *completer);
    void clearStationCallsignItems();
    void clearOperatorCallsignItems();
    QList<QVariant> getStationCallsignItems(const QString &call);
    QList<QVariant> getStationOperatorItems(const QString &call);
    void ClearCallbookFields();
    void SaveCallsignState();
    QComboBox *stationCallsign;
    QComboBox *operatorCallsign;
    cat_Interface *CAT;

    void setStationCallsignCurrentIndex(int idx);
    void setStationOperatorCurrentIndex(int idx);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

signals:
    void saveQSO();
    void updateDB();
    void stationCallsignIndexChanged(int);
    void operatorCallsignIndexChanged(int);
    void bandTextChanged(const QString &arg1);
    void modeTextChanged(const QString &arg1);
    void bandIndexChanged(int);
    void findCallTimer();
    void freqTextChanged(const QString &arg1);
    void gridSquareTextEdited(const QString &arg1);
    void CNTYTextEdited(const QString &arg1);
    void freqEditingFinished();
    void rstrEditingFinished();
    void rstsEditingFinished();

private slots:
    void toggleDock();
    void UpdateFormDateTime();
    void CallsignToUppercase(const QString &arg);
    void on_SaveQsoButton_clicked();
    void on_RefreshButton_clicked();
    void on_ClearQsoButton_clicked();
    void on_StationCallsignCurrentIndexChanged(int);
    void on_BandComboCurrentIndexChanged(int index);
    void on_OperatorCallsignCurrentIndexChanged(int index);
    void on_BandComboCurrentTextChanged(const QString &arg1);
    void on_ModeComboCurrentTextChanged(const QString &arg1);
    void on_FreqInputTextChanged(const QString &arg1);
    void on_GridSquareInputTextChanged(const QString &arg1);
    void on_CNTYInputTextChanged(const QString &arg1);
    void on_FreqInputEditingFinished();
    void on_RstrInputEditingFinished();
    void on_RstsInputEditingFinished();

private:
    DockPosition detectDock();
    void tryDock();
    void showDockIndicator(DockPosition pos);
    void hideDockIndicator();
    void minimizePanel();
    QWidget* makeInputPair(const QString &labelText, QWidget *inputWidget);
    QWidget* makeLabelWidgetPair(const QString &labelText, QWidget *Widget1, QWidget *Widget2);
    QWidget* makeWidgetPair(QWidget *Widget1, QWidget *Widget2);
    void SaveFormData();


private:
    QMainWindow *m_mainWindow;
    Settings *settings;
    bool m_docked;
    bool m_dragging;
    bool left_btn_pressed;
    QPoint m_dragPosition;
    DockPosition m_dockPos;

    FlowLayout *flowLayout1;
    FlowLayout *flowLayout2;
    FlowLayout *flowLayout3;
    FlowLayout *flowLayout4;
    FlowLayout *flowLayout5;
    FlowLayout *flowLayout6;

    QLineEdit *QTHLocEdit;
    QComboBox *BandCombo;
    QComboBox *ModeCombo;
    QLineEdit *RDAEdit;
    QLineEdit *FreqInput;
    QDateEdit *DateEdit;
    QTimeEdit *TimeEdit;
    QCheckBox *ShowCurrentTime;

    QLineEdit *CallInput;
    QLabel *countryFlag;
    QLabel *countryName;
    QLineEdit *NameInput;
    QLineEdit *QTHInput;
    QLineEdit *RstsInput;
    QLineEdit *RstrInput;
    QLineEdit *GridSquareInput;
    QLineEdit *CNTYInput;

    QLineEdit *CommentInput;
    QLabel *QSOSUserIcon;
    QLabel *QSOSUserLabel;
    QLabel *UserSRRIcon;
    QLabel *UserSRRLabel;

    QTimer *EverySecondTimer;
    QTimer *CallTypeTimer;
    QFrame *m_titleBar;
    QLabel *m_titleLabel;
    QPushButton *m_closeBtn;
    QWidget *m_content;

    bool m_resizing;
    bool m_resizeTop, m_resizeBottom, m_resizeLeft, m_resizeRight;
    QPoint m_resizeStartPos;
    QRect m_resizeStartGeo;
    QRubberBand *m_rubberBand; // Индикатор дока
    Ui::qsopanel ui; // UI из Designer
    QList<QWidget*> flowLayoutWidgets; // все динамические виджеты в FlowLayout
    QMap<QLabel*, QString> dynamicLabelsText;
};

#endif // QSOPANEL_H
