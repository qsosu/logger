/**********************************************************************************************************
Description :  Implementation of the WaitSpinner class, which provides a lightweight animated waiting
            :  indicator for Qt Widgets applications. The class renders a rotating circular sequence
            :  of fading dots using QPainter and QTimer, without relying on external resources such as
            :  GIF images or QML components.
Version     :  1.0.0
Date        :  25.12.2025
Author      :  R9JAU
Comments    :  - Designed to be used as an overlay widget for indicating background operations
            :    (e.g. network requests, database queries, API calls).
            :  - Uses QPainter with antialiasing for smooth rendering of animated dots.
            :  - Animation is driven by QTimer, providing consistent frame updates in the GUI thread.
            :  - Automatically supports transparent backgrounds and does not intercept mouse events.
            :  - Can be started and stopped programmatically via start() / stop() methods.
            :  - Intended for use in Qt Widgets (QDialog / QWidget) and supports dynamic repositioning
            :    (e.g. centering within the parent widget on resize).
**********************************************************************************************************/

#include "waitspinner.h"
#include <QPainter>
#include <QtMath>

//--------------------------------------------------------------------------------------------------------------------

WaitSpinner::WaitSpinner(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(32, 32);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);

    timer.setInterval(50); // скорость вращения
    connect(&timer, &QTimer::timeout, this, [this] {
        angle = (angle + 30) % 360;
        update();
    });

    hide();
}
//--------------------------------------------------------------------------------------------------------------------

void WaitSpinner::start()
{
    angle = 0;
    show();
    timer.start();
}
//--------------------------------------------------------------------------------------------------------------------

void WaitSpinner::stop()
{
    timer.stop();
    hide();
}
//--------------------------------------------------------------------------------------------------------------------

void WaitSpinner::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int dots = 12;
    const int r = width() / 2 - 3;
    const QPoint center(width() / 2, height() / 2);

    for (int i = 0; i < dots; ++i) {
        float alpha = 1.0f - float(i) / dots;
        QColor c = palette().color(QPalette::Highlight);
        c.setAlphaF(alpha);

        p.setBrush(c);
        p.setPen(Qt::NoPen);

        float a = qDegreesToRadians(float(angle + i * (360 / dots)));
        QPointF pos(
            center.x() + qCos(a) * r,
            center.y() + qSin(a) * r
        );

        p.drawEllipse(pos, 3, 3);
    }
}
//--------------------------------------------------------------------------------------------------------------------



