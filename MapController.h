#include <QObject>
#include <QGeoCoordinate>

class MapController : public QObject
{
    Q_OBJECT
public:
    explicit MapController(QObject *parent = nullptr);
    void setMap(QObject *mapObj);

    Q_INVOKABLE void addMarker(const QString &id, double lat, double lon, const QString &label, const QString &iconSource);
    Q_INVOKABLE void removeMarker(const QString &id);
    Q_INVOKABLE void addPolyline(const QList<QGeoCoordinate> &coordinates, const QString &mcolor, int width);
    Q_INVOKABLE void clearMarkers();

private:
    QObject *m_map = nullptr;

public slots:
    void receiveDistance(double distance);
    void receiveAzimuth(double azimuth);
    void receiveCallLatLon(QGeoCoordinate callLatLon);

signals:
    void distanceChanged(QString dist_str);
    void azimuthChanged(QString azimut_str);
    void callLatLonChanged(QString callLatLon_str);

};
