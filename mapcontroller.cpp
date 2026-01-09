/**********************************************************************************************************
Description :  Implementation of the MapController class, which acts as a bridge between the C++ backend
            :  and the QML map object. Provides methods to add/remove markers, clear map objects,
            :  draw polylines (routes), and handle measurement data (distance, azimuth, coordinates).
Version     :  1.5.0
Date        :  05.03.2025
Author      :  R9JAU
Comments    :  - Uses QMetaObject::invokeMethod to call QML-side map functions dynamically.
            :  - Supports adding markers with custom labels and icons.
            :  - Supports drawing polyline paths with configurable color and width.
            :  - Provides formatted text signals for displaying distance, azimuth, and coordinates.
***********************************************************************************************************/


#include "mapcontroller.h"
#include <QMetaObject>
#include <QDebug>

MapController::MapController(QObject *parent) : QObject(parent) {}

//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::setMap(QObject *mapObj)
{
    m_map = mapObj;
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::addMarker(const QString &id, double lat, double lon, const QString &label, const QString &iconSource)
{
    if (!m_map) return;

    QMetaObject::invokeMethod(m_map, "addMarker",
        Q_ARG(QVariant, id),
        Q_ARG(QVariant, lat),
        Q_ARG(QVariant, lon),
        Q_ARG(QVariant, label),
        Q_ARG(QVariant, iconSource));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::removeMarker(const QString &id)
{
    if (!m_map) return;
    QMetaObject::invokeMethod(m_map, "removeMarker", Q_ARG(QVariant, id));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::clearMarkers()
{
    if (!m_map) return;
    QMetaObject::invokeMethod(m_map, "clearMarkers");
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::addPolyline(const QList<QGeoCoordinate> &coordinates, const QString &color, int width)
{
    if (!m_map) return;
    QVariantList coordList;
    for (const QGeoCoordinate &coord : coordinates) {
        coordList << QVariant::fromValue(coord);
    }

    QMetaObject::invokeMethod(m_map, "addLine",
          Q_ARG(QVariant, coordList),
          Q_ARG(QVariant, color),
          Q_ARG(QVariant, width));
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::receiveDistance(double distance)
{
    QString text = QString(tr("Расстояние: %1 км.")).arg(distance, 0, 'f', 0);
    emit distanceChanged(text);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::receiveAzimuth(double azimuth)
{
    double back_azimuth = int((azimuth + 180))%360;
    QString text = QString(tr("Азимут: %1/%2 град.")).arg(azimuth, 0, 'f', 0).arg(back_azimuth, 0, 'f', 0);
    emit azimuthChanged(text);
}
//------------------------------------------------------------------------------------------------------------------------------------------

void MapController::receiveCallLatLon(QGeoCoordinate callLatLon)
{
    QString text = QString(tr("Широта: %1'N  Долгота: %2'E")).arg(callLatLon.latitude(), 0, 'f', 2).arg(callLatLon.longitude(), 0, 'f', 2);
    emit callLatLonChanged(text);
}
//------------------------------------------------------------------------------------------------------------------------------------------

