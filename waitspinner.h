#ifndef WAITSPINNER_H
#define WAITSPINNER_H

#include <QWidget>
#include <QTimer>

class WaitSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit WaitSpinner(QWidget *parent = nullptr);

    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QTimer timer;
    int angle = 0;
};

#endif // WAITSPINNER_H
