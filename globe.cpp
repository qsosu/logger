/**********************************************************************************************************
Description :  Implementation of the Globe class — an OpenGL widget for rendering an interactive 3D globe.
            :  Supports textured Earth rendering, atmospheric glow, starfield, markers (balloons),
            :  great-circle arcs between points, and country borders loading from GeoJSON.
            :  Intended for visualizing QTH/QSO markers, building connection paths,
            :  and focusing the camera on selected objects.
Version     :  1.0.0
Date        :  24.09.2025
Author      :  R9JAU
Comments    :  - Earth texture is loaded from the provided path (texPath) or a default resource.
            :  - Markers are implemented as small spheres (balloons) with their own VBO/IBO,
            :    supporting add/remove/clear operations.
            :  - Country highlighting and selection are implemented via screen projection
            :    (GeoJSON contours -> NDC -> screen) — on click, a tooltip is displayed.
            :  - Direct support for: setQTHLocation, showQSO, clearMarkers — convenient methods
            :    for integration with logging applications.
            :  - Visual effects: atmosphere, stars (point sprites with adjustable size/brightness).
            :  - Camera control: rotation via mouse drag, zoom with mouse wheel,
            :    automatic centering on all markers (centerOnBalloons).
***********************************************************************************************************/

#include "globe.h"
#include <QImage>
#include <QPainter>
#include <QtMath>
#include <random>
#include <QLayout>
#include <QMainWindow>


Globe::Globe(const QString &texturePath, QWidget* parent)
    : QOpenGLWidget(parent), texPath(texturePath)
{
    setMinimumSize(500, 300);
    setFocusPolicy(Qt::StrongFocus);
}
//----------------------------------------------------------------------------------------------------------------

Globe::~Globe() {
    makeCurrent();
    if(vbo) glDeleteBuffers(1,&vbo);
    if(ibo) glDeleteBuffers(1,&ibo);
    if(textureId) glDeleteTextures(1,&textureId);
    for(auto &b:vboBalloons) glDeleteBuffers(1,&b);
    for(auto &b:iboBalloons) glDeleteBuffers(1,&b);
    if(vboLine) glDeleteBuffers(1,&vboLine);
    if(vboStars) glDeleteBuffers(1, &vboStars);
    if(starProgram) { delete starProgram; starProgram = nullptr; }

    delete program;
    delete lineProgram;
    doneCurrent();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::addBalloon(float lat, float lon, float size, const QVector3D &color, const QString &name)
{
    Balloon b { lat, lon, size, color, name };
    balloons.push_back(b);
    buildBalloons();
    update();
    //qDebug() << "ballun param: Name " << name << " Lat " << lat << " lon " << lon << " color " << color;
}
//----------------------------------------------------------------------------------------------------------------

bool Globe::removeBalloon(const QString &name)
{
    auto it = std::remove_if(balloons.begin(), balloons.end(),
                             [&](const Balloon &b){ return b.name == name; });
    if (it != balloons.end()) {
        balloons.erase(it, balloons.end());
        buildBalloons();
        update();
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------

void Globe::clearBalloons()
{
    balloons.clear();
    buildBalloons();
    update();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::addLine(float lat1, float lon1, float lat2, float lon2, const QVector3D &color)
{
    QVector3D a = latLonToXYZ(lat1, lon1, 1.01f);
    QVector3D b = latLonToXYZ(lat2, lon2, 1.01f);

    Line line { a, b, color };
    lines.push_back(line);
    update();
}
//----------------------------------------------------------------------------------------------------------------

bool Globe::removeLine(int index)
{
    if (index < 0 || index >= static_cast<int>(lines.size()))
        return false;
    lines.erase(lines.begin() + index);
    update();
    return true;
}
//----------------------------------------------------------------------------------------------------------------

void Globe::clearLines()
{
    lines.clear();
    update();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::centerOnBalloons()
{
    if(balloons.empty()) return;

    QVector3D center(0,0,0);
    for(const auto &b : balloons)
        center += latLonToXYZ(b.lat, b.lon, 1.0f);
    center /= balloons.size();

    rotY = -qRadiansToDegrees(atan2(center.x(), center.z()));

    QMatrix4x4 rotYMat;
    rotYMat.rotate(rotY, 0, 1, 0);
    QVector3D rotated = rotYMat * center;
    rotX = qRadiansToDegrees(atan2(rotated.y(), rotated.z()));
}
//----------------------------------------------------------------------------------------------------------------

void Globe::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    initShaders();
    buildGlobe();

    glEnable(GL_PROGRAM_POINT_SIZE); // Чтобы шейдер мог задавать gl_PointSize
    glEnable(GL_POINT_SPRITE);       // Старые константы
    buildStars();

    buildBalloons();
    glGenBuffers(1,&vboLine);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
    proj.setToIdentity();
    proj.perspective(45.0f,float(w)/float(qMax(1,h)),0.1f,100.0f);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::paintGL()
{
    glClearColor(0.08f,0.08f,0.10f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 model;
    model.rotate(rotX,1,0,0);
    model.rotate(rotY,0,1,0);
    QMatrix4x4 view;
    view.translate(0,0,-zoom);

    drawStars(model, view);
    drawGlobe(model, view);
    drawAtmosphere(model, view);
    drawBalloons(model, view);
    drawLines(model, view);
    drawLabels(model, view);
    drawCountries(model, view);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::mousePressEvent(QMouseEvent *e)
{
    lastPos=e->pos();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    QMatrix4x4 model;
    model.rotate(rotX,1,0,0);
    model.rotate(rotY,0,1,0);
    QMatrix4x4 view;
    view.translate(0,0,-zoom);

    highlightedCountry = -1; // Сброс подсветки
    QVector3D cameraPos(0,0,zoom);

    auto isVisible = [&](const QVector3D &p) {
        QVector3D posWorld = (model * QVector4D(p,1.0f)).toVector3D();
        QVector3D toCamera = (cameraPos - posWorld).normalized();
        return QVector3D::dotProduct(posWorld.normalized(), toCamera) > 0;
    };

    for(int ci=0; ci<countries.size(); ++ci){
        const auto &c = countries[ci];

        // Проверяем, видна ли хотя бы одна точка страны
        bool countryVisible = false;
        for(const auto &ring : c.contours){
            for(const auto &v : ring){
                if(isVisible(v)){ countryVisible = true; break; }
            }
            if(countryVisible) break;
        }
        if(!countryVisible) continue; // Страна за горизонтом — пропускаем

        bool clicked = false;
//        for(const auto &ring : c.contours){
//            QPolygon poly2D;
//            for(const auto &v : ring){
//                QVector4D clip = proj*view*model*QVector4D(v,1.0f);
//                if(clip.w() == 0) continue;
//                QVector3D ndc = clip.toVector3DAffine();
//                int x = (ndc.x()*0.5f + 0.5f) * width();
//                int y = (-ndc.y()*0.5f + 0.5f) * height();
//                poly2D << QPoint(x,y);
//            }
//            if(poly2D.containsPoint(pos, Qt::OddEvenFill)){
//                clicked = true;
//                break;
//            }
//        }

        if(clicked){
            highlightedCountry = ci;
            QToolTip::showText(e->globalPos(), c.name, this);
            break;
        }
    }
    update();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::mouseMoveEvent(QMouseEvent *e)
{
    QPoint dp = e->pos()-lastPos;
    if(e->buttons() & Qt::LeftButton){
        rotX += dp.y()*0.5f;
        rotY += dp.x()*0.5f;
        update();
    }
    lastPos = e->pos();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::wheelEvent(QWheelEvent *e)
{
    float delta = e->angleDelta().y()/120.0f;
    zoom -= delta*0.3f;
    zoom = qBound(1.5f, zoom, 10.0f);
    update();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::initShaders()
{
    program = new QOpenGLShaderProgram(this);
    const char *vsrc = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec2 aTex;
        uniform mat4 mvp;
        out vec2 vTex;
        void main(){ vTex = aTex; gl_Position = mvp*vec4(aPos,1.0); }
    )";
    const char *fsrc = R"(
        #version 330 core
        in vec2 vTex;
        out vec4 FragColor;
        uniform sampler2D tex;
        void main(){ FragColor = texture(tex,vTex); }
    )";
    program->addShaderFromSourceCode(QOpenGLShader::Vertex,vsrc);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,fsrc);
    program->link();

    lineProgram = new QOpenGLShaderProgram(this);
    const char *vsrcLine = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 mvp;
        void main(){ gl_Position = mvp * vec4(aPos,1.0); }
    )";
    const char *fsrcLine = R"(
        #version 330 core
        uniform vec3 color;
        out vec4 FragColor;
        void main(){ FragColor = vec4(color,1.0); }
    )";
    lineProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,vsrcLine);
    lineProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,fsrcLine);
    lineProgram->link();

    starProgram = new QOpenGLShaderProgram(this);
    const char *vsrcStar = R"(
        #version 330 core
        layout(location=0) in vec3 aPos;
        uniform mat4 mvp;
        uniform float pointSize;
        void main() {
            gl_Position = mvp * vec4(aPos, 1.0);
            gl_PointSize = pointSize;
        }
    )";
    const char *fsrcStar = R"(
        #version 330 core
        out vec4 FragColor;
        uniform float brightness;
        void main() {
            vec2 p = 2.0 * gl_PointCoord - vec2(1.0);
            float r = dot(p,p);
            float alpha = smoothstep(1.0, 0.3, r) * brightness;
            FragColor = vec4(vec3(1.0), alpha);
        }
    )";
    starProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrcStar);
    starProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrcStar);
    starProgram->link();

    atmoProgram = new QOpenGLShaderProgram(this);
    // Атмосфера — вершинный шейдер
    const char* atmoVSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        out vec3 vPos;
        void main() {
            vec4 worldPos = model * vec4(aPos, 1.0);
            vPos = worldPos.xyz;
            gl_Position = projection * view * worldPos;
        }
    )";

    // Атмосфера — фрагментный шейдер
    const char* atmoFSrc = R"(
        #version 330 core
        in vec3 vPos;
        out vec4 FragColor;
        uniform vec3 cameraPos;
        uniform vec3 colorInner;
        uniform vec3 colorOuter;

        void main() {
            vec3 normal = normalize(vPos);
            vec3 viewDir = normalize(cameraPos - vec3(0.0)); // центр Земли
            float intensity = pow(1.0 - dot(normal, viewDir), 3.0);
            vec3 color = mix(colorOuter, colorInner, intensity);
            FragColor = vec4(color, intensity);
        }
    )";
    atmoProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,atmoVSrc);
    atmoProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,atmoFSrc);
    atmoProgram->link();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::buildGlobe()
{
    buildSphere(verts, inds, 1.0f, 64,64);
    idxCount = inds.size();

    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1,&ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    // Текстура
    QImage img(texPath.isEmpty() ? ":resources/images/earth.jpg" : texPath);
    if(img.isNull()){
        QImage f(2,2,QImage::Format_RGB32); f.fill(Qt::blue); img=f;
    } else img = img.convertToFormat(QImage::Format_RGBA8888).mirrored(true,false);

    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D,textureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width(),img.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);    

    // Построим небольшую сферу чуть больше Земли
    buildSphere(atmoVerts, atmoInds, 1.05f, 64, 64);

    glGenBuffers(1, &vboAtmo);
    glBindBuffer(GL_ARRAY_BUFFER, vboAtmo);
    glBufferData(GL_ARRAY_BUFFER, atmoVerts.size() * sizeof(Vertex), atmoVerts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &iboAtmo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboAtmo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, atmoInds.size() * sizeof(unsigned int), atmoInds.data(), GL_STATIC_DRAW);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::buildStars()
{
    // Случайные точки на сфере
    stars.clear();
    stars.resize(starCount);
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> ud01(0.0f,1.0f);
    for(int i=0;i<starCount;++i){
        // Равномерное распределение по сфере (по направлению)
        float z = 2.0f*ud01(rng) - 1.0f;           // [-1,1]
        float phi = 2.0f * M_PI * ud01(rng);       // [0,2pi)
        float r = sqrtf(1.0f - z*z);
        float x = r * cosf(phi);
        float y = r * sinf(phi);
        stars[i] = QVector3D(x, y, z) * starFieldRadius;
    }

    if(vboStars == 0) glGenBuffers(1, &vboStars);
    glBindBuffer(GL_ARRAY_BUFFER, vboStars);
    glBufferData(GL_ARRAY_BUFFER, stars.size()*sizeof(QVector3D), stars.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::buildBalloons()
{
    for(const auto &b: balloons){
        std::vector<Vertex> bv; std::vector<unsigned int> bi;
        buildSphere(bv, bi, b.size, 8, 8);
        GLuint vb, ib;
        glGenBuffers(1,&vb);
        glBindBuffer(GL_ARRAY_BUFFER,vb);
        glBufferData(GL_ARRAY_BUFFER,bv.size()*sizeof(Vertex),bv.data(),GL_STATIC_DRAW);
        glGenBuffers(1,&ib);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,bi.size()*sizeof(unsigned int),bi.data(),GL_STATIC_DRAW);
        vboBalloons.push_back(vb);
        iboBalloons.push_back(ib);
        idxBalloons.push_back(bi.size());
    }
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawGlobe(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    program->bind();
    QMatrix4x4 mvp = proj * view * model;
    program->setUniformValue("mvp", mvp);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    program->setUniformValue("tex", 0);

    glDrawElements(GL_TRIANGLES, idxCount, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    program->release();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawAtmosphere(const QMatrix4x4 &model, const QMatrix4x4 &view) {
    if (!atmoProgram) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    atmoProgram->bind();
    atmoProgram->setUniformValue("model", model);
    atmoProgram->setUniformValue("view", view);
    atmoProgram->setUniformValue("projection", proj);
    atmoProgram->setUniformValue("cameraPos", QVector3D(0, 0, zoom));
    atmoProgram->setUniformValue("colorInner", QVector3D(0.5f, 0.5f, 1.0f)); // яркое свечение у края
    atmoProgram->setUniformValue("colorOuter", QVector3D(0.1f, 0.3f, 0.5f)); // рассеянное свечение

    glBindBuffer(GL_ARRAY_BUFFER, vboAtmo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboAtmo);
    atmoProgram->enableAttributeArray(0);
    atmoProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(Vertex));

    glDrawElements(GL_TRIANGLES, atmoInds.size(), GL_UNSIGNED_INT, nullptr);

    atmoProgram->disableAttributeArray(0);
    atmoProgram->release();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawStars(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    if(stars.empty() || vboStars == 0 || !starProgram) return;

    // Рисуем звёзды перед землёй, но не записываем в depth buffer,
    // чтобы они не закрывали Землю.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // не писать в z-буфер

    starProgram->bind();
    // model уже содержит rotX/rotY (в paintGL мы передаём модель с поворотами),
    // поэтому звёзды будут двигаться синхронно с поворотом глобуса.
    QMatrix4x4 mvp = proj * view * model;
    starProgram->setUniformValue("mvp", mvp);
    starProgram->setUniformValue("pointSize", starPointSize);
    starProgram->setUniformValue("brightness", 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vboStars);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), (void*)0);
    glDrawArrays(GL_POINTS, 0, stars.size());
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    starProgram->release();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawBalloons(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    auto isVisible = [&](const QVector3D &pos) {
        QVector3D posWorld = (model * QVector4D(pos, 1.0f)).toVector3D();
        QVector3D cameraPos(0,0,zoom);
        QVector3D toCamera = (cameraPos - posWorld).normalized();
        return QVector3D::dotProduct(posWorld.normalized(), toCamera) > 0;
    };

    for(size_t i=0; i<balloons.size(); ++i){
        const Balloon &b = balloons[i];
        QVector3D pos = latLonToXYZ(b.lat,b.lon,1.0f);
        if(!isVisible(pos)) continue;

        QMatrix4x4 modelB = model;
        modelB.translate(pos);
        QMatrix4x4 mvpB = proj*view*modelB;

        lineProgram->bind();
        lineProgram->setUniformValue("mvp", mvpB);
        lineProgram->setUniformValue("color", b.color);

        glBindBuffer(GL_ARRAY_BUFFER, vboBalloons[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboBalloons[i]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,x));
        glDrawElements(GL_TRIANGLES, idxBalloons[i], GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        lineProgram->release();
    }
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawLines(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    auto isVisible = [&](const QVector3D &pos){
        QVector3D posWorld = (model * QVector4D(pos,1.0f)).toVector3D();
        QVector3D cameraPos(0,0,zoom);
        QVector3D toCamera = (cameraPos - posWorld).normalized();
        return QVector3D::dotProduct(posWorld.normalized(), toCamera) > 0;
    };

    lineProgram->bind();
    QMatrix4x4 mvpLine = proj*view*model;
    lineProgram->setUniformValue("mvp", mvpLine);
    glLineWidth(3.0f);

    for(const auto &l : lines){
        std::vector<QVector3D> arc = buildArc(l.start,l.end,64);
        std::vector<QVector3D> visibleArc;
        for(const auto &p : arc) if(isVisible(p)) visibleArc.push_back(p);
        if(visibleArc.size()<2) continue;

        glBindBuffer(GL_ARRAY_BUFFER,vboLine);
        glBufferData(GL_ARRAY_BUFFER, visibleArc.size()*sizeof(QVector3D), visibleArc.data(), GL_DYNAMIC_DRAW);
        lineProgram->setUniformValue("color", l.color);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(QVector3D),(void*)0);
        glDrawArrays(GL_LINE_STRIP,0,visibleArc.size());
        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
    }
    lineProgram->release();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawLabels(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    QPainter painter(this);
    painter.setFont(QFont("Arial",8));

    auto isVisible = [&](const QVector3D &pos){
        QVector3D posWorld = (model * QVector4D(pos,1.0f)).toVector3D();
        QVector3D cameraPos(0,0,zoom);
        QVector3D toCamera = (cameraPos - posWorld).normalized();
        return QVector3D::dotProduct(posWorld.normalized(), toCamera) > 0;
    };

    for(const auto &b : balloons){
        QVector3D pos3D = latLonToXYZ(b.lat,b.lon,1.0f);
        if(!isVisible(pos3D)) continue;
        QVector4D clipPos = proj*view*model*QVector4D(pos3D,1.0f);
        if(clipPos.w()==0.0f) continue;
        QVector3D ndc = clipPos.toVector3DAffine();
        int x = (ndc.x()*0.5f + 0.5f) * width();
        int y = (-ndc.y()*0.5f + 0.5f) * height();

        QFontMetrics fm(painter.font());
        int textWidth = fm.horizontalAdvance(b.name);
        int textHeight = fm.height();

        painter.setPen(Qt::black);
        painter.drawText(x - textWidth/2 + 1, y - textHeight + 1, b.name);
        painter.setPen(Qt::white);
        painter.drawText(x - textWidth/2, y - textHeight, b.name);
    }
    painter.end();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::drawCountries(const QMatrix4x4 &model, const QMatrix4x4 &view)
{
    if (highlightedCountry < 0 || highlightedCountry >= countries.size()) return;
    const auto &c = countries[highlightedCountry];
    if (c.contours.empty()) return;

    QVector3D cameraPos(0,0,zoom);

    auto isVisible = [&](const QVector3D &pos) {
        QVector3D posWorld = (model * QVector4D(pos,1.0f)).toVector3D();
        QVector3D toCamera = (cameraPos - posWorld).normalized();
        return QVector3D::dotProduct(posWorld.normalized(), toCamera) > 0;
    };

    // Проверка, видна ли страна
    bool visible = false;
    for (const auto &ring : c.contours) {
        for (const auto &p : ring) {
            if (isVisible(p)) { visible = true; break; }
        }
        if (visible) break;
    }
    if (!visible) return;

     // Контур страны
    lineProgram->bind();
    lineProgram->setUniformValue("mvp", proj * view * model);
    lineProgram->setUniformValue("color", QVector3D(1.0f, 0.0f, 0.0f));
    glLineWidth(1.5f);
    for (const auto &ring : c.contours) {
        if (ring.size() < 2) continue;

        glBindBuffer(GL_ARRAY_BUFFER, vboLine);
        glBufferData(GL_ARRAY_BUFFER, ring.size()*sizeof(QVector3D), ring.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), 0);
        glDrawArrays(GL_LINE_LOOP, 0, ring.size());
        glDisableVertexAttribArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    lineProgram->release();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::buildSphere(std::vector<Vertex> &verts, std::vector<unsigned int> &inds, float R, int sectors, int stacks)
{
    verts.clear(); inds.clear();
    for(int i=0;i<=stacks;++i){
        float v = float(i)/stacks;
        float phi = (v-0.5f)*M_PI;
        for(int j=0;j<=sectors;++j){
            float u = float(j)/sectors;
            float theta = u*2*M_PI;
            float x = R*cos(phi)*cos(theta);
            float y = R*sin(phi);
            float z = R*cos(phi)*sin(theta);
            verts.push_back({x,y,z,u,1.0f-v});
        }
    }
    for(int i=0;i<stacks;++i){
        int k1=i*(sectors+1), k2=k1+sectors+1;
        for(int j=0;j<sectors;++j){
            if(i!=0){ inds.push_back(k1+j); inds.push_back(k2+j); inds.push_back(k1+j+1); }
            if(i!=(stacks-1)){ inds.push_back(k1+j+1); inds.push_back(k2+j); inds.push_back(k2+j+1);}
        }
    }
}
//----------------------------------------------------------------------------------------------------------------

QVector3D Globe::latLonToXYZ(float lat, float lon, float radius)
{
    float phi = qDegreesToRadians(90.0f - lat);
    float theta = qDegreesToRadians(180.0f - lon);
    float x = radius*sin(phi)*cos(theta);
    float y = radius*cos(phi);
    float z = radius*sin(phi)*sin(theta);
    return QVector3D(x,y,z);
}
//----------------------------------------------------------------------------------------------------------------

std::vector<QVector3D> Globe::buildArc(const QVector3D &a, const QVector3D &b, int segments)
{
    std::vector<QVector3D> arc; arc.reserve(segments+1);
    float dot = QVector3D::dotProduct(a.normalized(),b.normalized());
    dot = qBound(-1.0f,dot,1.0f);
    float theta = acos(dot);
    for(int i=0;i<=segments;++i){
        float t=float(i)/segments;
        float sinT = sin((1-t)*theta);
        float sinT1 = sin(t*theta);
        QVector3D p=(a*sinT + b*sinT1)/sin(theta);
        arc.push_back(p);
    }
    return arc;
}
//----------------------------------------------------------------------------------------------------------------

void Globe::clearMarkers()
{
    clearBalloons();
    clearLines();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::showQSO(const QString &name, float lat, float lon)
{
    //clearMarkers();
    setQTHLocation(qthName, qthLatitude, qthLongitude);
    addBalloon(lat, lon, 0.02f, QVector3D(0,1,0), name);
    addLine(qthLatitude, qthLongitude, lat, lon, QVector3D(1,1,0));
    centerOnBalloons();
}
//----------------------------------------------------------------------------------------------------------------

void Globe::setQTHLocation(const QString &name, float lat, float lon)
{
    qthLatitude = lat;
    qthLongitude = lon;
    qthName = name;
    addBalloon(lat, lon, 0.02f, QVector3D(0,1,0), name);
}
//----------------------------------------------------------------------------------------------------------------

// Метод для загрузки GeoJSON
void Globe::loadCountries(const QString &filePath) {
    QFile f(filePath);
    if(!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if(!doc.isObject()) return;

    QJsonArray features = doc.object().value("features").toArray();
    for(const auto &featVal : features){
        QJsonObject feat = featVal.toObject();
        QString name = feat["properties"].toObject().value("name").toString();
        QJsonObject geom = feat["geometry"].toObject();
        QString type = geom["type"].toString();

        Country c; c.name = name;

        if(type=="Polygon"){
            QJsonArray rings = geom["coordinates"].toArray();
            for(const auto &ringVal : rings){
                std::vector<QVector3D> ringVec;
                QJsonArray ring = ringVal.toArray();
                for(const auto &ptVal : ring){
                    QJsonArray pt = ptVal.toArray();
                    double lon = pt[0].toDouble();
                    double lat = pt[1].toDouble();
                    ringVec.push_back(latLonToXYZ(lat, lon, 1.001f));
                }
                c.contours.push_back(ringVec);
            }
        } else if(type=="MultiPolygon"){
            QJsonArray polys = geom["coordinates"].toArray();
            for(const auto &polyVal : polys){
                QJsonArray rings = polyVal.toArray();
                for(const auto &ringVal : rings){
                    std::vector<QVector3D> ringVec;
                    QJsonArray ring = ringVal.toArray();
                    for(const auto &ptVal : ring){
                        QJsonArray pt = ptVal.toArray();
                        double lon = pt[0].toDouble();
                        double lat = pt[1].toDouble();
                        ringVec.push_back(latLonToXYZ(lat, lon, 1.001f));
                    }
                    c.contours.push_back(ringVec);
                }
            }
        }
        countries.push_back(c);
    }
}
//----------------------------------------------------------------------------------------------------------------


