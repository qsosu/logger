#ifndef GLOBE_H
#define GLOBE_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QString>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include <vector>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QToolTip>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QtMath>

struct Vertex { float x,y,z; float u,v; };
struct Balloon { float lat, lon, size; QVector3D color; QString name; };
struct Line { QVector3D start, end; QVector3D color; };

struct Country {
    QString name;
    std::vector<std::vector<QVector3D>> contours; // каждый полигон или кольцо как отдельный вектор
};


class Globe : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit Globe(const QString &texturePath = "", QWidget* parent = nullptr);
    ~Globe() override;

    void addBalloon(float lat, float lon, float size, const QVector3D &color, const QString &name);
    bool removeBalloon(const QString &name);
    void clearBalloons();
    void addLine(float lat1, float lon1, float lat2, float lon2, const QVector3D &color);
    bool removeLine(int index);
    void clearLines();
    void centerOnBalloons();
    void clearMarkers();
    void showQSO(const QString &name, float lat, float lon);
    void setQTHLocation(const QString &name, float lat, float lon);
    void loadCountries(const QString &filePath);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    void initShaders();
    void buildGlobe();
    void buildBalloons();
    void buildStars();
    void drawGlobe(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawAtmosphere(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawStars(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawBalloons(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawLines(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawLabels(const QMatrix4x4 &model, const QMatrix4x4 &view);
    void drawCountries(const QMatrix4x4 &model, const QMatrix4x4 &view);

    void buildSphere(std::vector<Vertex> &verts, std::vector<unsigned int> &inds, float R, int sectors, int stacks);
    QVector3D latLonToXYZ(float lat, float lon, float radius);
    std::vector<QVector3D> buildArc(const QVector3D &a,const QVector3D &b,int segments);

private:
    QOpenGLShaderProgram *program = nullptr;
    QOpenGLShaderProgram *lineProgram = nullptr;
    QOpenGLShaderProgram *starProgram = nullptr;
    QOpenGLShaderProgram *atmoProgram = nullptr;

    GLuint vboAtmo = 0, iboAtmo = 0;
    GLuint vbo = 0, ibo = 0, vboLine = 0, textureId = 0, vboStars = 0;

    std::vector<Vertex> atmoVerts;
    std::vector<unsigned int> atmoInds;

    std::vector<GLuint> vboBalloons, iboBalloons;
    std::vector<int> idxBalloons;

    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;
    std::vector<Balloon> balloons;
    std::vector<Line> lines;
    std::vector<Country> countries;
    std::vector<QVector3D> stars;

    int idxCount = 0;
    int idxAtmo = 0;
    float rotX = 0, rotY = 0, zoom = 3.0f;
    int highlightedCountry = -1; // индекс подсвеченной страны, -1 = нет
    QPoint lastPos;
    QMatrix4x4 proj;
    QString texPath;

    double qthLatitude = 0;
    double qthLongitude = 0;
    QString qthName;

    int starCount = 1500;            // можно менять — количество звёзд
    float starFieldRadius = 20.0f;   // радиус сферы звёзд (должен быть >> zoom)
    float starPointSize = 2.0f;      // размер точки (пиксели)
};

class GlobeContainer : public QWidget
{
    Q_OBJECT
public:
    GlobeContainer(const QString &texturePath, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        // Основной вертикальный layout
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);

        // OpenGL виджет
        globe = new Globe(texturePath, this);
        layout->addWidget(globe);

        // Статусбар
        status = new QStatusBar(this);
        status->setMaximumHeight(17);

        m_distance = new QLabel(tr("Расстояние: 0 км."));
        m_azimuth = new QLabel(tr("Азимут: 0 град."));
        m_callLatLon = new QLabel(tr("Широта: 0'N  Долгота: 0'E"));

        status->addPermanentWidget(m_distance);
        status->addPermanentWidget(m_azimuth);
        status->addPermanentWidget(m_callLatLon, 1);

        layout->addWidget(status);
        setLayout(layout);
    }

    void setDistanceText(const QString &text) { m_distance->setText(text); }
    void setAzimuthText(const QString &text) { m_azimuth->setText(text); }
    void setLatLonText(const QString &text) { m_callLatLon->setText(text); }

    Globe* globe;
    QStatusBar* status;
    QLabel *m_distance, *m_azimuth, *m_callLatLon;
    static constexpr double EARTH_RADIUS_KM = 6371.0; // Радиус Земли в километрах


    double calculateAzimuth(double lat1, double lon1, double lat2, double lon2)
    {
        // Переводим градусы в радианы
        double phi1 = qDegreesToRadians(lat1);
        double phi2 = qDegreesToRadians(lat2);
        double deltaLambda = qDegreesToRadians(lon2 - lon1);

        // Формула прямого геодезического азимута
        double y = qSin(deltaLambda) * qCos(phi2);
        double x = qCos(phi1) * qSin(phi2) - qSin(phi1) * qCos(phi2) * qCos(deltaLambda);

        double theta = qAtan2(y, x);        // результат в радианах
        double azimuth = qRadiansToDegrees(theta); // перевод в градусы

        // Приводим к диапазону 0-360°
        if (azimuth < 0) azimuth += 360;
        return azimuth;
    }

    double reverseAzimuth(double azimuth)
    {
        double rev = azimuth + 180.0;
        if(rev >= 360.0) rev -= 360.0;
        return rev;
    }

    double haversineDistance(double lat1, double lon1, double lat2, double lon2)
    {
        // Переводим градусы в радианы
        double latRad1 = qDegreesToRadians(lat1);
        double lonRad1 = qDegreesToRadians(lon1);
        double latRad2 = qDegreesToRadians(lat2);
        double lonRad2 = qDegreesToRadians(lon2);

        double dLat = latRad2 - latRad1;
        double dLon = lonRad2 - lonRad1;

        double a = qPow(qSin(dLat / 2), 2) +
                   qCos(latRad1) * qCos(latRad2) * qPow(qSin(dLon / 2), 2);

        double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));

        return EARTH_RADIUS_KM * c;
    }

    void clearMarkers()
    {
        m_distance->setText(tr("Расстояние: 0 км."));
        m_azimuth->setText(tr("Азимут: 0 град."));
        m_callLatLon->setText(tr("Широта: 0'N  Долгота: 0'E"));
        globe->clearMarkers();
    }
};

#endif // GLOBE_H
