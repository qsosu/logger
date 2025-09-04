#ifndef REPORTSUNCHART_H
#define REPORTSUNCHART_H

#include <QDialog>
#include <QtCharts>
#include <QSqlRecord>
#include <QSqlQuery>

struct DataPoint {
    qint64 timestamp;
    double value;
};

namespace Ui {
class ReportSunChart;
}

class ReportSunChart : public QDialog
{
    Q_OBJECT

public:
    explicit ReportSunChart(QSqlDatabase db, QWidget *parent = nullptr);
    ~ReportSunChart();
    void showMagneticStormChart();

private:
    Ui::ReportSunChart *ui;
    QGraphicsScene *scene;
    QWidget *legendOverlay = nullptr;
    QSqlDatabase db;
    QVector<QPair<QDateTime, int>> magneticData;

    QVector<QPointF> allPoints;
    QStringList allLabels;
    int pointSpacing = 30;
    int margin = 50;
    int chartTop = 50;
    int chartHeight = 0;
    int chartBottom = 0;

    QVector<QColor> levelColors = {
         QColor(0, 128, 0, 160), QColor(0, 128, 0, 160),
         QColor(255, 165, 0, 160), QColor(255, 165, 0, 160),
         QColor(255, 100, 0, 160), QColor(255, 100, 0, 160),
         QColor(255, 0, 0, 160), QColor(255, 0, 0, 160)
    };

    void drawChart();
    void updateVisibleChart();

public slots:
    void InsertMeasurement(QJsonArray data);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // REPORTSUNCHART_H
