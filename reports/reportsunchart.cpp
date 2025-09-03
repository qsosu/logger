#include "reportsunchart.h"
#include "ui_reportsunchart.h"
#include <climits>
#include <cfloat>
#include <QSqlError>
#include <QProgressDialog>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QtCharts>

using namespace QtCharts;

ReportSunChart::ReportSunChart(QSqlDatabase db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportSunChart)
{
    ui->setupUi(this);
    this->db = db;

    Qt::WindowFlags flags = this->windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    flags |= Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;
    this->setWindowFlags(flags);

    legendOverlay = nullptr;

    connect(ui->graphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ReportSunChart::updateVisibleChart);
}

ReportSunChart::~ReportSunChart()
{
    delete ui;
}

//--------------------------------------------------------------------------------------------------------------------

void ReportSunChart::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    drawChart(); // пересчёт по новой высоте
}
//--------------------------------------------------------------------------------------------------------------------

// Загрузка данных и показ графика
void ReportSunChart::showMagneticStormChart()
{
    QSqlQuery query("SELECT measurement_time, value FROM magnetic_storm ORDER BY measurement_time DESC");
    QVector<QPair<QDateTime, int>> data;

    // Прогресс-бар
    QProgressDialog progress("Загрузка данных...", "", 0, query.size());
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    progress.setMinimumDuration(0);
    progress.setCancelButton(nullptr);
    progress.setValue(0);

    int j = 0;
    while (query.next()) {
        QDateTime dt = QDateTime::fromString(query.value(0).toString(), Qt::ISODate);
        int value = query.value(1).toInt();
        data.append({dt, value});

        progress.setValue(++j);
        QCoreApplication::processEvents();
    }
    progress.close();

    if (data.isEmpty()) return;

    magneticData = data; // сохраняем для динамического ресайза
    drawChart();

    // Обработка скролла
    connect(ui->graphicsView->horizontalScrollBar(), &QScrollBar::valueChanged,
            this, &ReportSunChart::updateVisibleChart, Qt::UniqueConnection);

    // Скроллим к последним точкам
    QTimer::singleShot(0, this, [this]() {
        ui->graphicsView->horizontalScrollBar()->setValue(
        ui->graphicsView->horizontalScrollBar()->maximum());
    });
}
//--------------------------------------------------------------------------------------------------------------------

// Основная функция рисования графика
void ReportSunChart::drawChart()
{
    if (magneticData.isEmpty() || !ui->graphicsView)
        return;

    const int topMargin = 50;      // Отступ сверху
    const int bottomMargin = 100;  // Отступ снизу для подписей X
    chartTop = topMargin;
    chartHeight = ui->graphicsView->height() - topMargin - bottomMargin;
    chartBottom = chartTop + chartHeight;

    allPoints.clear();
    allLabels.clear();

    double minVal = 0, maxVal = 9;
    int pointCount = magneticData.size();

    // Рассчёт точек графика
    for (int i = 0; i < pointCount; ++i) {
        int reversedIndex = pointCount - 1 - i;
        const auto &entry = magneticData[reversedIndex];
        int value = entry.second;

        double x = margin + i * pointSpacing;
        double y = chartBottom - ((value - minVal) / (maxVal - minVal)) * chartHeight;

        allPoints.append(QPointF(x, y));
        allLabels << entry.first.toString("yyyy-MM-dd HH:mm");
    }

    updateVisibleChart();
}
//--------------------------------------------------------------------------------------------------------------------

// Отрисовка только видимой части + запас
void ReportSunChart::updateVisibleChart()
{
    const int bottomMargin = 100;  // Отступ снизу для подписей X
    int sceneHeight = chartTop + chartHeight + bottomMargin; // Полная высота сцены

    if (allPoints.isEmpty() || !ui->graphicsView) return;

    QGraphicsScene *scene = new QGraphicsScene(this);

    int viewWidth = ui->graphicsView->viewport()->width();
    int hScroll = ui->graphicsView->horizontalScrollBar()->value();

    int firstIndex = qMax(0, (hScroll - margin) / pointSpacing - 20);
    int lastIndex  = qMin(allPoints.size() - 1, (hScroll + viewWidth - margin) / pointSpacing + 20);

    // Градиентная кривая
    for (int i = firstIndex; i < lastIndex; ++i) {
        if (i + 1 >= allPoints.size()) break;

        QPointF p0 = allPoints[i];
        QPointF p1 = allPoints[i + 1];

        int val0 = magneticData[magneticData.size() - 1 - i].second;
        int val1 = magneticData[magneticData.size() - 1 - (i + 1)].second;

        QColor c0 = levelColors[qBound(0, val0 - 1, 7)];
        QColor c1 = levelColors[qBound(0, val1 - 1, 7)];

        QLinearGradient grad(p0, p1);
        grad.setColorAt(0, c0);
        grad.setColorAt(1, c1);

        QPen segPen(QBrush(grad), 2);
        segPen.setCapStyle(Qt::RoundCap);
        segPen.setJoinStyle(Qt::RoundJoin);

        QPointF c1pt(p0.x() + (p1.x() - p0.x()) / 2.0, p0.y());
        QPointF c2pt(p0.x() + (p1.x() - p0.x()) / 2.0, p1.y());

        QPainterPath segPath(p0);
        segPath.cubicTo(c1pt, c2pt, p1);

        scene->addPath(segPath, segPen);
    }

    // Точки
    const int pointRadius = 4;
    for (int i = firstIndex; i <= lastIndex; ++i) {
        int reversedIndex = magneticData.size() - 1 - i;
        if (reversedIndex < 0 || reversedIndex >= magneticData.size()) continue;

        const auto &entry = magneticData[reversedIndex];
        int value = entry.second;

        QGraphicsEllipseItem *dot = scene->addEllipse(
            allPoints[i].x() - pointRadius,
            allPoints[i].y() - pointRadius,
            pointRadius * 2,
            pointRadius * 2,
            QPen(Qt::NoPen),
            QBrush(levelColors[qBound(0, value - 1, 7)])
        );
        dot->setToolTip(QString("Время: %1\nУровень: %2")
                        .arg(entry.first.toString("yyyy-MM-dd HH:mm"))
                        .arg(value));
    }

    // Оси
    QPen axisPen(QColor(180, 180, 180));
    scene->addLine(margin, chartTop, margin, chartBottom, axisPen); // вертикальная ось
    scene->addLine(margin, chartBottom, margin + pointSpacing * allPoints.size(), chartBottom, axisPen); // горизонтальная ось

    for (int i = 0; i <= 9; ++i) {
        double y = chartBottom - (i / 9.0) * chartHeight;
        scene->addLine(margin - 5, y, margin, y, axisPen);
        QGraphicsTextItem *label = scene->addText(QString::number(i), QFont("Arial", 8, QFont::Bold));
        label->setDefaultTextColor(QColor(180, 180, 180));
        label->setPos(margin - 35, y - 10);
    }

    // Подписи оси X
    for (int i = firstIndex; i <= lastIndex; ++i) {
        double x = allPoints[i].x();
        scene->addLine(x, chartBottom, x, chartBottom + 5, axisPen);
        QGraphicsTextItem *label = scene->addText(allLabels[i], QFont("Arial", 8, QFont::Bold));
        label->setRotation(45);
        label->setDefaultTextColor(QColor(180, 180, 180));
        label->setPos(x, chartBottom + 10);
    }

    // Легенда
    if (legendOverlay) {
        legendOverlay->deleteLater();
        legendOverlay = nullptr;
    }

    legendOverlay = new QWidget(ui->graphicsView->parentWidget());
    legendOverlay->raise();
    legendOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    legendOverlay->setStyleSheet("background: transparent;");
    legendOverlay->setFixedWidth(220);

    QTimer::singleShot(0, this, [this]() {
        if (!legendOverlay || !ui->graphicsView) return;
        QRect gvRect = ui->graphicsView->geometry();
        legendOverlay->adjustSize();
        legendOverlay->move(gvRect.right() - legendOverlay->width() - 10, gvRect.top() + 10);
        legendOverlay->show();
    });

    QVBoxLayout *layout = new QVBoxLayout(legendOverlay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    QStringList legendTexts = {
        "1 — Нет заметных возмущений.",
        "2 — Небольшие возмущения.",
        "3 — Слабая геомагнитная буря.",
        "4 — Малая геомагнитная буря.",
        "5 — Умеренная геомагнитная буря.",
        "6 — Сильная геомагнитная буря.",
        "7 — Жесткий геомагнитный шторм.",
        "8 — Экстремальный шторм."
    };

    for (int i = 0; i < legendTexts.size(); ++i) {
        QWidget *item = new QWidget(legendOverlay);
        QHBoxLayout *itemLayout = new QHBoxLayout(item);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(5);

        QLabel *colorBox = new QLabel(item);
        colorBox->setFixedSize(10, 10);
        colorBox->setStyleSheet(QString("background-color: rgba(%1,%2,%3,%4);")
                                .arg(levelColors[i].red())
                                .arg(levelColors[i].green())
                                .arg(levelColors[i].blue())
                                .arg(levelColors[i].alpha()));

        QLabel *text = new QLabel(legendTexts[i], item);
        text->setStyleSheet("color: rgb(150,150,150); font: 8pt 'Arial';");

        itemLayout->addWidget(colorBox);
        itemLayout->addWidget(text);
        layout->addWidget(item);
    }

    scene->setSceneRect(0, 0, margin * 2 + pointSpacing * allPoints.size(), sceneHeight);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
//--------------------------------------------------------------------------------------------------------------------

// Вставка измерений в базу
void ReportSunChart::InsertMeasurement(QJsonArray data)
{
    QSqlQuery query(db);
    const int measCount = data.size();

    QProgressDialog progress("Загрузка данных.", "", 0, measCount);
    progress.setWindowTitle("Пожалуйста, подождите");
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);
    progress.setCancelButton(nullptr);
    progress.setValue(0);

    for (int i = 0; i < measCount; ++i) {
        QJsonObject obj = data[i].toObject();
        int id = obj["id"].toInt();
        QString timeStr = obj["measurement_time"].toString();
        double value = obj["value"].toDouble();

        query.prepare("INSERT INTO MAGNETIC_STORM (id, measurement_time, value) "
                      "VALUES (:id, :measurement_time, :value)");
        query.bindValue(":id", id);
        query.bindValue(":measurement_time", timeStr);
        query.bindValue(":value", value);

        query.exec(); // игнорируем ошибки

        progress.setValue(i + 1);
        QCoreApplication::processEvents();
    }
    progress.close();
    showMagneticStormChart();
}
//--------------------------------------------------------------------------------------------------------------------
