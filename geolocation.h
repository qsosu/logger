#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <QDialog>
#include <QQuickWidget>
#include <QLabel>
#include <qstatusbar.h>
#include "mapcontroller.h"

#include <QQuickView>
#include <QGeoCoordinate>

struct Coordinates {
    double latitude;
    double longitude;
};

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Geolocation;
}
QT_END_NAMESPACE

//------------------------------------------------------------------------------------------------------------------------------------------

class Geolocation : public QDialog
{
    Q_OBJECT
public:
    explicit Geolocation(QWidget *parent = nullptr);
    ~Geolocation();
    double qth_lat;
    double qth_lng;
    QString qth_call;

    void showQSOMap(QString call, double lat, double lng);
    void setQSOMarker(QString call, double lat, double lng);
    void setQTHLocation(QString call, double lat, double lng);
    void clearMarkers();
    Coordinates locatorToCoordinates(const QString& locator);

private:
    Ui::Geolocation *ui;
    QQuickView *map_view;
    MapController controller;
    QStatusBar *statusbar;
    QLabel *m_distance;
    QLabel *m_azimuth;
    QLabel *m_callLatLon;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
};
//------------------------------------------------------------------------------------------------------------------------------------------

#endif // GEOLOCATION_H
