/**********************************************************************************************************
Description :  Implementation of the Geolocation class, which provides a dialog window with an interactive
            :  map for visualizing QSOs. The class allows setting QTH location, displaying markers for
            :  contacts, drawing connection paths, and calculating distance/azimuth between points.
            :  Includes conversion from Maidenhead Locator to geographic coordinates.
Version     :  1.0.0
Date        :  12.04.2025
Author      :  R9JAU
Comments    :  Uses QQuickWidget with MapView QML for rendering the map. Integrates MapController
            :  to manage markers, polylines, and coordinate calculations.
***********************************************************************************************************/


#include "geolocation.h"
#include "ui_geolocation.h"
#include <QQuickItem>
#include <QQmlContext>
#include <QEvent>
#include <QTimer>
#include <cmath>


//------------------------------------------------------------------------------------------------------------------------------------------

Geolocation::Geolocation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Geolocation)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Карта связей"));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint);

    // Status bar
    statusbar = new QStatusBar(this);
    ui->gridLayout->addWidget(statusbar, 1, 0);
    statusbar->setMaximumHeight(17);
    statusbar->layout()->setContentsMargins(0,0,0,0);

    m_distance = new QLabel(tr("Расстояние: 0 км."));
    statusbar->insertPermanentWidget(0, m_distance, 0);

    m_azimuth = new QLabel(tr("Азимут: 0 град."));
    statusbar->insertPermanentWidget(1, m_azimuth, 0);

    m_callLatLon = new QLabel(tr("Широта: 0'N  Долгота: 0'E"));
    statusbar->insertPermanentWidget(2, m_callLatLon, 1);

    // QQuickWidget для карты
    QQuickWidget *mapWidget = new QQuickWidget(this);
    mapWidget->rootContext()->setContextProperty("mapController", &controller);
    mapWidget->setSource(QUrl(QStringLiteral("qrc:/MapView.qml")));
    mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->gridLayout->addWidget(mapWidget, 0, 0);

    // Подключаем сигналы после того, как QML дерево построено
    QTimer::singleShot(0, this, [=]() {
        QQuickItem *rootObject = mapWidget->rootObject();
        if (!rootObject) {
            qWarning() << "Root object null!";
            return;
        }

        QObject *map = rootObject->findChild<QObject*>("map");
        if (!map) {
            qWarning() << "Map object not found! Проверь objectName в QML";
            return;
        }
        controller.setMap(map);

        connect(&controller, &MapController::distanceChanged, m_distance, &QLabel::setText);
        connect(&controller, &MapController::azimuthChanged, m_azimuth, &QLabel::setText);
        connect(&controller, &MapController::callLatLonChanged, m_callLatLon, &QLabel::setText);
    });
}
//------------------------------------------------------------------------------------------------------------------------------------------

Geolocation::~Geolocation()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::showQSOMap(QString call, double lat, double lng)
{
    controller.clearMarkers();
    controller.addMarker("1", qth_lat, qth_lng, qth_call, "qrc:///resources/marker.png");
    controller.addMarker("2", lat, lng, call, "qrc:///resources/marker.png");

    QList<QGeoCoordinate> path;
    path.clear();
    path << QGeoCoordinate(qth_lat, qth_lng) << QGeoCoordinate(lat, lng);
    controller.addPolyline(path, "blue", 1);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::setQSOMarker(QString call, double lat, double lng)
{
    controller.addMarker("1", lat, lng, call, "qrc:///resources/marker.png");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::clearMarkers()
{
    controller.clearMarkers();
    m_distance->setText(tr("Расстояние: 0 км."));
    m_azimuth->setText(tr("Азимут: 0 град."));
    m_callLatLon->setText(tr("Широта: 0'N  Долгота: 0'E"));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::setQTHLocation(QString call, double lat, double lng)
{
    qth_lat = lat;
    qth_lng = lng;
    qth_call = call;
    controller.addMarker("0", qth_lat, qth_lng, qth_call, "qrc:///resources/marker.png");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::closeEvent(QCloseEvent *event)
{
    controller.clearMarkers();
    QDialog::closeEvent(event);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void Geolocation::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------
// Функция для преобразования Maidenhead Locator в географические координаты
Coordinates Geolocation::locatorToCoordinates(const QString& locator)
{
    QString l = locator.toUpper();
    int len = l.length();
    if (len < 2 || len % 2 != 0 || len > 12)
        return {0.0, 0.0};

    double lon = -180.0;
    double lat = -90.0;

    int pairs = len / 2;

    for (int i = 0; i < pairs; ++i) {
        QChar lonChar = l[i * 2];
        QChar latChar = l[i * 2 + 1];

        int lonVal = 0;
        int latVal = 0;
        double lonStep = 0.0;
        double latStep = 0.0;

        switch (i) {
        case 0: // Field (A-R)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 20.0;
            latStep = 10.0;
            break;
        case 1: // Square (0-9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0;
            latStep = 1.0;
            break;
        case 2: // Subsquare (A-X)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 2.0 / 24.0;    // 0.083333...
            latStep = 1.0 / 24.0;    // 0.041666...
            break;
        case 3: // Extended square (0–9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0 / 240.0;   // 0.008333...
            latStep = 1.0 / 240.0;   // 0.004166...
            break;
        case 4: // Extended subsquare (A–X)
            lonVal = lonChar.toLatin1() - 'A';
            latVal = latChar.toLatin1() - 'A';
            lonStep = 2.0 / 5760.0;  // 0.0003472...
            latStep = 1.0 / 5760.0;  // 0.0001736...
            break;
        case 5: // Microsquare (0–9)
            lonVal = lonChar.toLatin1() - '0';
            latVal = latChar.toLatin1() - '0';
            lonStep = 2.0 / 57600.0; // 0.00003472...
            latStep = 1.0 / 57600.0; // 0.00001736...
            break;
        default:
            break;
        }
        lon += lonVal * lonStep;
        lat += latVal * latStep;

        // На последнем уровне — добавляем половину размера ячейки, чтобы попасть в центр
        if (i == pairs - 1) {
            lon += lonStep / 2.0;
            lat += latStep / 2.0;
        }
    }
    return {lat, lon};
}
//------------------------------------------------------------------------------------------------------------------------------------------

