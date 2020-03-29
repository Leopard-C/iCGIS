#include "widget/openglwidget.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mapbox/earcut.hpp>

#include "opengl/vertexbufferlayout.h"
#include "opengl/openglfeaturedescriptor.h"
#include "geo/geometry/geogeometry.h"
#include "geo/utility/geo_utility.h"
#include "util/utility.h"
#include "util/env.h"
#include "util/appevent.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <QDebug>


/*************************************/
/*                                   */
/*         OpenGLWidget              */
/*                                   */
/*************************************/

OpenGLWidget::OpenGLWidget(QWidget* parent /*= 0*/)
    : QOpenGLWidget(parent), map(Env::map), opList(Env::opList)
{
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);

    connect(this, &OpenGLWidget::sigUpdateCoord,
            AppEvent::getInstance(), &AppEvent::onUpdateCoord);
    connect(AppEvent::getInstance(), &AppEvent::sigZoomToMap,
            this, &OpenGLWidget::onZoomToMap);
    connect(AppEvent::getInstance(), &AppEvent::sigZoomToLayer,
            this, &OpenGLWidget::onZoomToLayer);
    connect(AppEvent::getInstance(), &AppEvent::sigUpdateOpengl,
            this, [this]{ update(); });
    connect(AppEvent::getInstance(), &AppEvent::sigSendMapToGPU,
            this, &OpenGLWidget::onSendMapToGPU);
    connect(AppEvent::getInstance(), &AppEvent::sigSendLayerToGPU,
            this, &OpenGLWidget::onSendLayerToGPU);
    connect(AppEvent::getInstance(), &AppEvent::sigSendFeatureToGPU,
            this, &OpenGLWidget::onSendFeatureToGPU);
}

OpenGLWidget::~OpenGLWidget()
{
    // very important
    makeCurrent();
    isRunning = false;
}

/**********************************************/
/*                                            */
/*                  Override                  */
/*                                            */
/**********************************************/
void OpenGLWidget::initializeGL()
{
    if (glewInit() != GLEW_NO_ERROR) {
        std::cout << "Glew init failed" << std::endl;
        return;
    }

    Env::createShaders();

    // view matrix
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void OpenGLWidget::resizeGL(int w, int h) {
    GLCall(glViewport(0, 0, w, h));
    GLCall(glMatrixMode(GL_PROJECTION));

    if (!map || map->isEmpty())
        return;
    if (h < 1)
        h = 1;

    // projection matrix
    float aspectRatio = float(w) / h;
    GeoExtent mapExtent = map->getExtent();
    if (mapExtent.aspectRatio() < aspectRatio) {
        adjustedMapExtent.minX = (mapExtent.minX - mapExtent.centerX()) * aspectRatio / mapExtent.aspectRatio() + mapExtent.centerX();
        adjustedMapExtent.maxX = (mapExtent.maxX - mapExtent.centerX()) * aspectRatio / mapExtent.aspectRatio() + mapExtent.centerX();
    }
    else {
        adjustedMapExtent.minY = (mapExtent.minY - mapExtent.centerY()) * mapExtent.aspectRatio() / aspectRatio + mapExtent.centerY();
        adjustedMapExtent.maxY = (mapExtent.maxY - mapExtent.centerY()) * mapExtent.aspectRatio() / aspectRatio + mapExtent.centerY();
    }
    proj = glm::ortho(float(adjustedMapExtent.minX), float(adjustedMapExtent.maxX),
                      float(adjustedMapExtent.minY), float(adjustedMapExtent.maxY),
                      -1.0f, 1.0f);

    // MVP
    setMVP();

    update();
}

void OpenGLWidget::paintGL()
{
    Env::renderer.Clear();

    if (!isRunning || !map || map->isEmpty())
        return;

    map->Draw();

    if (isRectSelecting) {
        GLCall(glUseProgram(0));
        drawRectNoFill(mouseBeginPos, mouseCurrPos, 1.0f, 0.5f, 0.0f, 5);
        drawRectFillColor(mouseBeginPos, mouseCurrPos, 1.0f, 0.5f, 0.0f, 0.5f);
    }
}


/********************************/
/*         Unility Function     */
/********************************/

// WCS ==> SCS  (world/screen coordinate system)
// Just use occasionally
// When draw shapes, it will be processed by GPU
QPoint OpenGLWidget::xy2screen(double geoX, double geoY)
{
    // x:[-1, 1]  y:[-1, 1]  z:[-1, 1]
    float stdX = (2 * geoX + 2 * xOffset - (adjustedMapExtent.minX + adjustedMapExtent.maxX)) / adjustedMapExtent.width();
    float stdY = (2 * geoY + 2 * yOffset - (adjustedMapExtent.minY + adjustedMapExtent.maxY)) / adjustedMapExtent.height();

    float screenX = (stdX + 1) * this->width() / 2.0f;
    float screenY = this->height() - (stdY + 1) * this->height() / 2.0f;

    return { int(screenX), int(screenY) };
}


// SCS ==> WCS
GeoRawPoint OpenGLWidget::screen2xy(int screenX, int screenY)
{
    // x:[-1, 1]  y:[-1, 1]  z:[-1, 1]
    GeoRawPoint stdXY = screen2stdxy(screenX, screenY);

    float geoX = (stdXY.x * adjustedMapExtent.width() + adjustedMapExtent.minX + adjustedMapExtent.maxX - 2 * xOffset) / (2 * zoom);
    float geoY = (stdXY.y * adjustedMapExtent.height() + adjustedMapExtent.minY + adjustedMapExtent.maxY - 2 * yOffset) / (2 * zoom);

    return { geoX, geoY };
}


// SCS ==> NDC
GeoRawPoint OpenGLWidget::screen2stdxy(int screenX, int screenY)
{
    float stdX = (2.0f * screenX / this->width()) - 1;
    float stdY = (2.0f * (this->height() - screenY) / this->height()) - 1;
    return { stdX, stdY };
}

// Length in: SCS ==> WCS
double OpenGLWidget::getLengthInWorldSystem(int screenLength) {
    return adjustedMapExtent.width() * screenLength / (this->width() * zoom);
}

// Update MVP matrix
void OpenGLWidget::updateMVP(bool updateModel /*= true*/, bool updateView /*= true*/, bool updateProj /*= false*/)
{
    GeoExtent mapExtent = map->getExtent();
    adjustedMapExtent = mapExtent;

    // model matrix
    if (updateModel) {
        // model
        // translation
        xOffset = 0.0f;
        yOffset = 0.0f;
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
        // sacle
        zoom = 1.0f;
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
        model = trans * scale;
    }

    // view matrix
    if (updateView) {
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }

    // projection matrix
    if (updateProj) {
        float aspectRatio = float(this->width()) / this->height();
        if (mapExtent.aspectRatio() < aspectRatio) {
            adjustedMapExtent.minX = (mapExtent.minX - mapExtent.centerX()) * aspectRatio / mapExtent.aspectRatio() + mapExtent.centerX();
            adjustedMapExtent.maxX = (mapExtent.maxX - mapExtent.centerX()) * aspectRatio / mapExtent.aspectRatio() + mapExtent.centerX();
        }
        else {
            adjustedMapExtent.minY = (mapExtent.minY - mapExtent.centerY()) * mapExtent.aspectRatio() / aspectRatio + mapExtent.centerY();
            adjustedMapExtent.maxY = (mapExtent.maxY - mapExtent.centerY()) * mapExtent.aspectRatio() / aspectRatio + mapExtent.centerY();
        }
        proj = glm::ortho(float(adjustedMapExtent.minX), float(adjustedMapExtent.maxX),
                          float(adjustedMapExtent.minY), float(adjustedMapExtent.maxY),
                          -1.0f, 1.0f);
    }
}

void OpenGLWidget::setMVP()
{
    glm::mat4 mvp = proj * view * model;

    Env::pointShader.Bind();
    Env::pointShader.SetUniformMat4f("u_MVP", mvp);

    Env::lineShader.Bind();
    Env::lineShader.SetUniformMat4f("u_MVP", mvp);

    Env::polygonShader.Bind();
    Env::polygonShader.SetUniformMat4f("u_MVP", mvp);

    Env::borderShader.Bind();
    Env::borderShader.SetUniformMat4f("u_MVP", mvp);

    Env::highlightShader.Bind();
    Env::highlightShader.SetUniformMat4f("u_MVP", mvp);

    Env::textureShader.Bind();
    Env::textureShader.SetUniformMat4f("u_MVP", mvp);
}

/**************************************************/
/*                                                */
/*              Event                             */
/*                                                */
/**************************************************/

/***************************************/
/*            Mouse enter              */
/***************************************/
void OpenGLWidget::enterEvent(QEvent*) {
    qDebug() << "enter";
    switch (Env::cursorType) {
    case Env::CursorType::Normal: {
        setCursor(Qt::ArrowCursor);
        break;
    }
    case Env::CursorType::Editing: {
        QCursor editingCursor(QPixmap("res/icons/editing-arrow.ico"), 4, 1);
        setCursor(editingCursor);
        break;
    }
    case Env::CursorType::Palm: {
        setCursor(Qt::OpenHandCursor);
        break;
    }
    case Env::CursorType::WhatIsThis: {
        QCursor whatIsThisCursor(QPixmap("res/icons/what-is-this.png"), 2, 4);
        setCursor(whatIsThisCursor);
        break;
    }
    }
}

/***************************************/
/*            Press key                */
/***************************************/
void OpenGLWidget::keyPressEvent(QKeyEvent *ev) {
    qDebug() << "Key press: " << ev->key();
    makeCurrent();
    switch (ev->key()) {
    case Qt::Key_Delete: {
        std::map<GeoFeatureLayer*, std::vector<GeoFeature*>> selectedFeatures;
        map->getAllSelectedFeatures(selectedFeatures);
        if (selectedFeatures.empty())
            break;
        opList.addDeleteOperation(selectedFeatures)->operate();
        clearSelected();
        break;
    }
    case Qt::Key_Z: {
        if (ev->modifiers() != Qt::ControlModifier)
            break;
        opList.undo();
        break;
    }
    case Qt::Key_R: {
        if (ev->modifiers() != Qt::ControlModifier)
            break;
        opList.redo();
        break;
    }
    default:
        return;
    }
    update();
}


/***************************************/
/*            Mouse Press              */
/***************************************/
void OpenGLWidget::mousePressEvent(QMouseEvent* ev)
{
    mouseBeginPos = mouseLastPos = ev->pos();
    isMouseClicked = true;

    /* What is This */
    if (Env::cursorType == Env::CursorType::WhatIsThis) {
        clearSelected();
        return;
    }

    /* Whether move selected features */
    if (Env::isEditing && Env::cursorType == Env::CursorType::Editing && isSelected) {
        GeoRawPoint geoXY = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
        double halfEdge = getLengthInWorldSystem(5);
        GeoFeatureLayer* featureLayer = nullptr;
        GeoFeature* feature = nullptr;
        map->queryFeature(geoXY.x, geoXY.y, halfEdge, featureLayer, feature);
        if (featureLayer && feature && feature->isSelected()) {
            isMovingFeatures = true;
            return;
        }
    }

    if (Env::isEditing && Env::cursorType == Env::CursorType::Editing) {
        clearSelected();
    }
}

/***************************************/
/*            Mouse Move               */
/***************************************/
void OpenGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
    mouseCurrPos = ev->pos();

    if (!map || map->isEmpty())
        return;

    // If mouse's left button was not pressed,
    //  update current coordinate in WCS under the cursor
    if (!isMouseClicked) {
        GeoRawPoint geoXY = screen2xy(ev->x(), ev->y());
        emit sigUpdateCoord(geoXY.x, geoXY.y);
        return;
    }

    if (Env::cursorType == Env::CursorType::WhatIsThis) {
        GeoRawPoint geoXY = screen2xy(ev->x(), ev->y());
        emit sigUpdateCoord(geoXY.x, geoXY.y);
        return;
    }

    makeCurrent();

    // Moving featrues
    if (isMovingFeatures && Env::cursorType == Env::CursorType::Editing) {
        GeoRawPoint lastGeoXY = screen2xy(mouseLastPos.x(), mouseLastPos.y());
        GeoRawPoint currGeoXY = screen2xy(mouseCurrPos.x(), mouseCurrPos.y());
        emit sigUpdateCoord(currGeoXY.x, currGeoXY.y);
        map->offsetSelectedFeatures(currGeoXY.x - lastGeoXY.x, currGeoXY.y - lastGeoXY.y);
        mouseLastPos = mouseCurrPos;
        update();
        return;
    }

    // Editing fetures
    if (Env::isEditing && Env::cursorType == Env::CursorType::Editing) {
        // the distance of the mouse moves
        if ((mouseCurrPos - mouseBeginPos).manhattanLength() < 6)
            return;
        // Draw rectangle dynamically
        isRectSelecting = true;
        update();
    }

    // Moving the map
    else {
        GeoRawPoint stdLastPos = screen2stdxy(mouseLastPos.x(), mouseLastPos.y());
        GeoRawPoint stdCurrPos = screen2stdxy(mouseCurrPos.x(), mouseCurrPos.y());
        xOffset += ((stdCurrPos.x - stdLastPos.x) * adjustedMapExtent.width()) / 2.0;
        yOffset += ((stdCurrPos.y - stdLastPos.y) * adjustedMapExtent.height()) / 2.0;

        // model matrix
        // translation
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
        model = trans * scale;

        // MVP
        setMVP();

        mouseLastPos = mouseCurrPos;
        update();
    }
}

/***************************************/
/*            Mouse Release            */
/***************************************/
void OpenGLWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    QPoint mouseCurrPos = ev->pos();
    isRectSelecting = false;
    isMouseClicked = false;

    if (!map || map->isEmpty())
        return;

    /* What is This */
    if (Env::cursorType == Env::CursorType::WhatIsThis) {
        GeoRawPoint geoXY = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
        double halfEdge = getLengthInWorldSystem(5);
        GeoFeatureLayer* featureLayer = nullptr;
        GeoFeature* feature = nullptr;
        map->queryFeature(geoXY.x, geoXY.y, halfEdge, featureLayer, feature);
        if (featureLayer && feature) {
            map->emplaceSelectedFeature(featureLayer->getLID(), feature);
            if (!whatIsThisDialog) {
                whatIsThisDialog = new WhatIsThisDialog(this);
                connect(whatIsThisDialog, &WhatIsThisDialog::closed, this, [this]{
                    this->whatIsThisDialog = nullptr;
                });
                whatIsThisDialog->show();
            }
            whatIsThisDialog->setFeature(featureLayer, feature);
        }
        update();
        return;
    }

    // Moving featrues
    if (isMovingFeatures) {
        GeoRawPoint beginGeoXY = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
        GeoRawPoint endGeoXY = screen2xy(mouseCurrPos.x(), mouseCurrPos.y());
        double offsetX = endGeoXY.x - beginGeoXY.x;
        double offsetY = endGeoXY.y - beginGeoXY.y;
        std::map<GeoFeatureLayer*, std::vector<GeoFeature*>> selectedFeatures;
        map->getAllSelectedFeatures(selectedFeatures);
        opList.addMoveOperation(selectedFeatures, offsetX, offsetY);
        for (auto& f1 : selectedFeatures) {
            f1.first->updateExtent();
            f1.first->createGridIndex();    // update spatial index
        }
        isMovingFeatures = false;
        update();
        return;
    }

    // Editing
    if (Env::isEditing && Env::cursorType == Env::CursorType::Editing) {
        // Point selection
        if ((mouseCurrPos - mouseBeginPos).manhattanLength() < 6) {
            GeoRawPoint geoXY = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
            double halfEdge = getLengthInWorldSystem(8);
            GeoFeatureLayer* featureLayer = nullptr;
            GeoFeature* feature = nullptr;
            map->queryFeature(geoXY.x, geoXY.y, halfEdge, featureLayer, feature);
            if (featureLayer && feature) {
                map->emplaceSelectedFeature(featureLayer->getLID(), feature);
                isSelected = true;
            }
        }
        // Box selection
        else {
            GeoRawPoint leftTop = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
            GeoRawPoint rightBottom = screen2xy(mouseCurrPos.x(), mouseCurrPos.y());
            GeoExtent rect(leftTop, rightBottom);
            rect.normalize();   // normalize
            std::map<GeoFeatureLayer*, std::vector<GeoFeature*>> selectedFeatures;
            map->queryFeatures(rect, selectedFeatures);
            if (selectedFeatures.size() > 0) {
                map->setSelectedFeatures(selectedFeatures);
                isSelected = true;
            }
        }
        update();
        return;
    } // end if isEditing

    QOpenGLWidget::mouseReleaseEvent(ev);
}

/***************************************/
/*            Mouse Wheel              */
/***************************************/
void OpenGLWidget::wheelEvent(QWheelEvent* ev)
{
    GeoRawPoint geoXY = screen2xy(ev->x(), ev->y());

    if (!map || map->isEmpty())
        return;

    // formula
    //  Delta(xOffset) = (zoomOld - zoomNew) * zoomCenterX;
    //  Delta(yOffset) = (zoomOld - zoomNew) * zoomCenterY;

    // zoom in
    if (ev->delta() > 0) {
        xOffset += -0.1 * zoom * geoXY.x;
        yOffset += -0.1 * zoom * geoXY.y;
        zoom *= 1.1;

        // model matrix
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
        model = trans * scale;
    }
    // zoom out
    else {
        xOffset += 0.1 / 1.1 * zoom * geoXY.x;
        yOffset += 0.1 / 1.1 * zoom * geoXY.y;
        zoom /= 1.1;

        // model matrix
        trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
        model = trans * scale;
    }

    // MVP
    setMVP();
    update();
}

/**************************************************/
/*                                                */
/*                     Slot                       */
/*                                                */
/**************************************************/

// zoom to layer
void OpenGLWidget::onZoomToLayer(GeoLayer* layer)
{
    GeoExtent layerExtent = layer->getExtent();

    if (layerExtent.aspectRatio() < adjustedMapExtent.aspectRatio())
        zoom = adjustedMapExtent.height() / layerExtent.height();
    else
        zoom = adjustedMapExtent.width() / layerExtent.width();

    xOffset = adjustedMapExtent.centerX() - layerExtent.centerX();
    yOffset = adjustedMapExtent.centerY() - layerExtent.centerY();
    xOffset += (1.0f - zoom) * layerExtent.centerX();
    yOffset += (1.0f - zoom) * layerExtent.centerY();

    // model matrix
    trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
    model = trans * scale;

    setMVP();
    update();
}

void OpenGLWidget::onZoomToMap() {
    updateMVP(true, false, true);
    setMVP();
    update();
}


/**************************************************/
/*                                                */
/*            private functions                   */
/*                                                */
/**************************************************/

void OpenGLWidget::clearSelected()
{
    // get opengl contex
    makeCurrent();

    //openglLayerManager->clearSelected();
    map->clearSelectedFeatures();

    update();
}

// Draw rectangle width fill color
void OpenGLWidget::drawRectFillColor(const QPoint& startPoint, const QPoint& endPoint, float r, float g, float b, float a)
{
    GeoRawPoint leftTop = screen2stdxy(startPoint.x(), startPoint.y());
    GeoRawPoint rightBottom = screen2stdxy(endPoint.x(), endPoint.y());

    // Blend (Transparent display)
    bool usingBlend = fabs(a - 1.0f) < 0.001f ? false : true;
    if (usingBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glColor4f(r, g, b, a);
    glRectd(leftTop.x, leftTop.y, rightBottom.x, rightBottom.y);

    if (usingBlend) {
        glDisable(GL_BLEND);
    }
}

// Draw rectangel with border only
void OpenGLWidget::drawRectNoFill(const QPoint& startPoint, const QPoint& endPoint, float r /*= 0.0f*/, float g /*= 0.0f*/, float b /*= 0.0f*/, int lineWidth /*= 1*/)
{
    GeoRawPoint leftTop = screen2stdxy(startPoint.x(), startPoint.y());
    GeoRawPoint rightBottom = screen2stdxy(endPoint.x(), endPoint.y());
    glBegin(GL_LINE_STRIP);
    glColor3f(r, g, b);
    glLineWidth(lineWidth);
    glVertex2d(leftTop.x, leftTop.y);
    glVertex2d(leftTop.x, rightBottom.y);
    glVertex2d(rightBottom.x, rightBottom.y);
    glVertex2d(rightBottom.x, leftTop.y);
    glVertex2d(leftTop.x, leftTop.y);
    glEnd();
}


/*********************************************/
/*                                           */
/*        (re)Send all geoMap to GPU         */
/*                                           */
/*********************************************/

void OpenGLWidget::onSendMapToGPU(bool bUpdate) {
    for (auto iter = map->begin(); iter != map->end(); ++iter)
        onSendLayerToGPU(*iter, false);
    if (bUpdate)
        update();
}


/*********************************************/
/*                                           */
/*        SEND raster data to GPU            */
/*                                           */
/*********************************************/

void OpenGLWidget::onSendLayerToGPU(GeoLayer *layer, bool bUpdate) {
    if (layer->getLayerType() == LayerType::kFeatureLayer)
        onSendFeatureLayerToGPU(layer->toFeatureLayer(), bUpdate);
    else
        onSendRasterLayerToGPU(layer->toRasterLayer(), bUpdate);
}

void OpenGLWidget::onSendRasterLayerToGPU(GeoRasterLayer *rasterLayer, bool bUpdate)
{
    if (!rasterLayer)
        return;

    makeCurrent();

    GeoExtent extent = rasterLayer->getExtent();
    GeoRasterData* rasterData = rasterLayer->getData();

    for (auto bandIter = rasterData->begin(); bandIter != rasterData->end(); ++bandIter) {
        OpenglRasterDescriptor* rasterDesc = new OpenglRasterDescriptor();
        (*bandIter)->setOpenglRasterDescriptor(rasterDesc);

        auto& vao = rasterDesc->vao;
        auto& vbo = rasterDesc->vbo;
        auto& ibo = rasterDesc->ibo;
        auto& texs = rasterDesc->texs;

        // VAO
        vao = new VertexArray();

        // VBO
        float vertices[] = {
            // position					            // texture coords
            float(extent.maxX), float(extent.minY),   1.0f, 1.0f,
            float(extent.maxX), float(extent.maxY),   1.0f, 0.0f,
            float(extent.minX), float(extent.maxY),   0.0f, 0.0f,
            float(extent.minX), float(extent.minY),   0.0f, 1.0f
        };
        vbo = new VertexBuffer(vertices, 16 * sizeof(float));

        // Data layout
        VertexBufferLayout layout;
        layout.Push<float>(2);	// x, y
        layout.Push<float>(2);	// coordX, coordY
        vao->addBuffer(*vbo, layout);

        // IBO
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };
        ibo = new IndexBuffer(indices, 6, GL_TRIANGLES);

        // Texture
        GeoRasterData* rasterData = rasterLayer->getData();
        int bandsCount = rasterData->getBandsCount();
        for (int i = 0; i < bandsCount; ++i) {
            GeoRasterBand* band = rasterData->getBand(i);
            Texture* texture = nullptr;
            switch (band->getDataType()) {
            default:
                break;
            case utils::DataType::kFloat:
            {
                int count = band->width * band->height;
                // normalize to: [0, 1]
                float maxValue = *(std::max_element((float*)(band->pData), (float*)(band->pData) + count));
                float* pixels = (float*)(band->pData);
                float* buff = new float[count];
                for (int i = 0; i < count; ++i) {
                    buff[i] = pixels[i] / maxValue;
                }
                texture = new Texture(buff, band->width, band->height,
                                      GL_FLOAT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
                delete[] buff;
                texs.push_back(texture);
                break;
            }
            }
        }
    }

    if (map->getNumLayers() == 1)
        updateMVP(true, false, true);
    else
        updateMVP(false, false, true);

    setMVP();

    if (bUpdate) {
        update();
    }
}


/*********************************************/
/*                                           */
/*        SEND vector data to GPU            */
/*                                           */
/*********************************************/

void OpenGLWidget::onSendFeatureLayerToGPU(GeoFeatureLayer* featureLayer, bool bUpdate)
{
    if (!featureLayer)
        return;
    makeCurrent();

    for (auto featureIter = featureLayer->begin(); featureIter != featureLayer->end(); ++featureIter) {
        onSendFeatureToGPU(*featureIter);
    }

    if (map->getNumLayers() == 1)
        updateMVP(true, false, true);
    else
        updateMVP(false, false, true);

    setMVP();

    if (bUpdate) {
        update();
    }
}


void OpenGLWidget::onSendFeatureToGPU(GeoFeature *feature) {
    GeometryType geomType = feature->getGeometryType();

    float r, g, b;
    feature->getColorF(r, g, b);

    makeCurrent();
    OpenglFeatureDescriptor* featureDesc = nullptr;

    switch (geomType) {
    default:
        break;
    case kPoint:
    {
        GeoPoint* point = feature->getGeometry()->toPoint();
        featureDesc = sendPointToGPU(point, r, g, b);
        break;
    }
    case kPolygon:
    {
        GeoPolygon* polygon = feature->getGeometry()->toPolygon();
        featureDesc = sendPolygonToGPU(polygon, r, g, b);
        break;
    }
    case kLineString:
    {
        GeoLineString* lineString = feature->getGeometry()->toLineString();
        featureDesc = sendLineStringToGPU(lineString, r, g, b);
        break;
    }
    case kMultiPoint:
    {
        GeoMultiPoint* multiPoint = feature->getGeometry()->toMultiPoint();
        featureDesc = sendMultiPointToGPU(multiPoint, r, g, b);
        break;
    }
    case kMultiPolygon:
    {
        GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
        featureDesc = sendMultiPolygonToGPU(multiPolygon, r, g, b);
        break;
    }
    case kMultiLineString:
    {
        GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
        featureDesc = sendMultiLineStringToGPU(multiLineString, r, g, b);
        break;
    }
    }

    feature->setOpenglFeatureDescriptor(featureDesc);
}

OpenglFeatureDescriptor* OpenGLWidget::sendPointToGPU(GeoPoint* point, float r, float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(5);
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();

    // VBO
    float vertices[5] = { float(point->getX()), float(point->getY()), r, g, b };
    vbo = new VertexBuffer(vertices, 5 * sizeof(float));

    // IBO
    unsigned int indices = 0;
    IndexBuffer* ibo = new IndexBuffer(&indices, 1, GL_POINTS);
    ibos.emplace_back(ibo);

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}

OpenglFeatureDescriptor* OpenGLWidget::sendMultiPointToGPU(GeoMultiPoint* mutliPoint, float r, float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(5);
    int pointsCount = mutliPoint->getNumGeometries();
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();

    // VBO
    float* vertices = new float[pointsCount * 5 * sizeof(float)];
    GeoPoint* point;
    for (int i = 0; i < pointsCount; ++i) {
        point = mutliPoint->getGeometry(i)->toPoint();
        vertices[i * 5] = point->getX();
        vertices[i * 5 + 1] = point->getY();
        vertices[i * 5 + 2] = r;
        vertices[i * 5 + 3] = g;
        vertices[i * 5 + 4] = b;
    }
    vbo = new VertexBuffer(vertices, pointsCount * 5 * sizeof(float));

    // IBO
    unsigned int* indices = utils::newContinuousNumber(0, pointsCount);
    IndexBuffer* ibo = new IndexBuffer(indices, pointsCount, GL_POINTS);
    ibos.emplace_back(ibo);
    delete[] vertices;
    delete[] indices;

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}

OpenglFeatureDescriptor* OpenGLWidget::sendLineStringToGPU(GeoLineString* lineString, float r, float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(5);
    int pointsCount = lineString->getNumPoints();
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();

    // VBO
    float* vertices = new float[pointsCount * 5];
    for (int i = 0; i < pointsCount; ++i) {
        vertices[i * 5] = lineString->getX(i);
        vertices[i * 5 + 1] = lineString->getY(i);
        vertices[i * 5 + 2] = r;
        vertices[i * 5 + 3] = g;
        vertices[i * 5 + 4] = b;
    }
    vbo = new VertexBuffer(vertices, 5 * sizeof(float) * pointsCount);

    // IBO
    unsigned int* indices = utils::newContinuousNumber(0, pointsCount);
    IndexBuffer* ibo = new IndexBuffer(indices, pointsCount, GL_LINE_STRIP);
    ibos.emplace_back(ibo);
    delete[] vertices;
    delete[] indices;

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}

OpenglFeatureDescriptor* OpenGLWidget::sendMultiLineStringToGPU(GeoMultiLineString* multiLineString, float r, float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(5);
    int pointsCount = multiLineString->getNumPoints();
    int linesCount = multiLineString->getNumGeometries();
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();

    // VBO
    vbo = new VertexBuffer(nullptr, 5 * sizeof(float) * pointsCount);

    int sizeOffset = 0;
    int countOffset = 0;

    for (int i = 0; i < linesCount; ++i) {
        GeoLineString* lineString = multiLineString->getLineString(i);

        float* vertices = new float[pointsCount * 5];
        for (int i = 0; i < pointsCount; ++i) {
            vertices[i * 5] = lineString->getX(i);
            vertices[i * 5 + 1] = lineString->getY(i);
            vertices[i * 5 + 2] = r;
            vertices[i * 5 + 3] = g;
            vertices[i * 5 + 4] = b;
        }
        vbo->addSubData(vertices, sizeOffset, pointsCount * 5 * sizeof(float));

        // IBO
        unsigned int* indices = utils::newContinuousNumber(countOffset, pointsCount);
        IndexBuffer* ibo = new IndexBuffer(indices, pointsCount, GL_LINE_STRIP);
        ibos.push_back(ibo);
        sizeOffset += pointsCount * 5 * sizeof(float);
        countOffset += pointsCount;
        delete[] vertices;
        delete[] indices;
    }

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}

OpenglFeatureDescriptor* OpenGLWidget::sendPolygonToGPU(GeoPolygon* geoPolygon, float r,float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(8);
    int polygonPointsCount = geoPolygon->getNumPoints();
    int interiorRingsCount = geoPolygon->getInteriorRingsCount();
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();
    vao->reserve(interiorRingsCount + 1);

    float* vertices = new float[polygonPointsCount * 8 * sizeof(float)];
    int iStride = 0;

    using Point = std::array<double, 2>;
    std::vector<std::vector<Point>> polygon;

    // Exterior ring
    GeoLinearRing* geoExteriorRing = geoPolygon->getExteriorRing();
    int exteriorRingPointsCount = geoExteriorRing->getNumPoints();
    vao->setStride(iStride++, exteriorRingPointsCount);
    std::vector<Point> exteriorRing;
    exteriorRing.reserve(exteriorRingPointsCount);
    GeoRawPoint rawPoint;
    int index = 0;
    for (int i = 0; i < exteriorRingPointsCount; ++i) {
        geoExteriorRing->getRawPoint(i, &rawPoint);
        exteriorRing.push_back({ rawPoint.x, rawPoint.y });
        // position
        vertices[index] = rawPoint.x;
        vertices[index + 1] = rawPoint.y;
        // fill color
        vertices[index + 2] = r;
        vertices[index + 3] = g;
        vertices[index + 4] = b;
        // border color
        // default: black border
        vertices[index + 5] = 0.0f;
        vertices[index + 6] = 0.0f;
        vertices[index + 7] = 0.0f;
        index += 8;
    }
    polygon.emplace_back(exteriorRing);

    // Interior rings
    for (int j = 0; j < interiorRingsCount; ++j) {
        const auto& geoInteriorRing = geoPolygon->getInteriorRing(j);
        int interiorRingPointsCount = geoInteriorRing->getNumPoints();
        vao->setStride(iStride++, interiorRingPointsCount);
        std::vector<Point> interiorRing;
        interiorRing.reserve(interiorRingPointsCount);
        for (int k = 0; k < interiorRingPointsCount; ++k) {
            geoInteriorRing->getRawPoint(k, &rawPoint);
            exteriorRing.push_back({ rawPoint.x, rawPoint.y });
            // position
            vertices[index] = rawPoint.x;
            vertices[index + 1] = rawPoint.y;
            // fill color
            vertices[index + 2] = r;
            vertices[index + 3] = g;
            vertices[index + 4] = b;
            // border color
            // default: black border
            vertices[index + 5] = 0.0f;
            vertices[index + 6] = 0.0f;
            vertices[index + 7] = 0.0f;
            index += 8;
        }
        polygon.emplace_back(interiorRing);
    }

    // Triangulation
    std::vector<unsigned int> indices = mapbox::earcut<unsigned int>(polygon);
    //std::reverse(indices.begin(), indices.end());

    // VBO
    vbo = new VertexBuffer(vertices, polygonPointsCount * 8 * sizeof(float));

    // IBO
    IndexBuffer* ibo = new IndexBuffer(&indices[0], indices.size(), GL_TRIANGLES);
    ibos.push_back(ibo);
    delete[] vertices;

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// Fill   color: r, g, b
    layout.Push<float>(3);	// Border color: r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}


OpenglFeatureDescriptor* OpenGLWidget::sendMultiPolygonToGPU(GeoMultiPolygon* multiPolygon, float r,float g, float b)
{
    OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(8);
    int pointsCount = multiPolygon->getNumPoints();
    int polygonCount = multiPolygon->getNumGeometries();
    int linearRingsCount = multiPolygon->getNumLinearRings();
    auto& vao = featureDesc->vao;
    auto& vbo = featureDesc->vbo;
    auto& ibos = featureDesc->ibos;

    // VAO
    vao = new VertexArray();
    vao->reserve(linearRingsCount);

    int sizeOffset = 0;
    int countOffset = 0;

    // VBO
    // Just allocate memory, no data was send.
    vbo = new VertexBuffer(nullptr, pointsCount * 8 * sizeof(float));
    int iStride = 0;

    for (int i = 0; i < polygonCount; ++i) {
        GeoPolygon* geoPolygon = multiPolygon->getPolygon(i);
        int polygonPointsCount = geoPolygon->getNumPoints();
        int interiorRingsCount = geoPolygon->getInteriorRingsCount();

        float* vertices = new float[polygonPointsCount * 5 * sizeof(float)];

        using Point = std::array<double, 2>;
        std::vector<std::vector<Point>> polygon;

        // Exterior ring
        GeoLinearRing* geoExteriorRing = geoPolygon->getExteriorRing();
        int exteriorRingPointsCount = geoExteriorRing->getNumPoints();
        vao->setStride(iStride++, exteriorRingPointsCount);
        std::vector<Point> exteriorRing;
        exteriorRing.reserve(exteriorRingPointsCount);
        GeoRawPoint rawPoint;
        int index = 0;
        for (int i = 0; i < exteriorRingPointsCount; ++i) {
            geoExteriorRing->getRawPoint(i, &rawPoint);
            exteriorRing.push_back({ rawPoint.x, rawPoint.y });
            // position
            vertices[index] = rawPoint.x;
            vertices[index + 1] = rawPoint.y;
            // fill color
            vertices[index + 2] = r;
            vertices[index + 3] = g;
            vertices[index + 4] = b;
            // border color
            // default: black border
            vertices[index + 5] = 0.0f;
            vertices[index + 6] = 0.0f;
            vertices[index + 7] = 0.0f;
            index += 8;
        }
        polygon.emplace_back(exteriorRing);

        // Interior rings
        for (int j = 0; j < interiorRingsCount; ++j) {
            const auto& geoInteriorRing = geoPolygon->getInteriorRing(j);
            int interiorRingPointsCount = geoInteriorRing->getNumPoints();
            vao->setStride(iStride++, interiorRingPointsCount);
            std::vector<Point> interiorRing;
            interiorRing.reserve(interiorRingPointsCount);
            for (int k = 0; k < interiorRingPointsCount; ++k) {
                geoInteriorRing->getRawPoint(k, &rawPoint);
                exteriorRing.push_back({ rawPoint.x, rawPoint.y });
                // position
                vertices[index] = rawPoint.x;
                vertices[index + 1] = rawPoint.y;
                // fill color
                vertices[index + 2] = r;
                vertices[index + 3] = g;
                vertices[index + 4] = b;
                // border color
                // default: black border
                vertices[index + 5] = 0.0f;
                vertices[index + 6] = 0.0f;
                vertices[index + 7] = 0.0f;
                index += 8;
            }
            polygon.emplace_back(interiorRing);
        }

        // Triangulation
        std::vector<unsigned int> indices = mapbox::earcut<unsigned int>(polygon);
        //std::reverse(indices.begin(), indices.end());

        if (countOffset > 0)
            std::for_each(indices.begin(), indices.end(), [&countOffset](auto& value) {value += countOffset; });

        // VBO
        vbo->addSubData(vertices, sizeOffset, polygonPointsCount * 8 * sizeof(float));

        // IBO
        IndexBuffer* ibo = new IndexBuffer(&indices[0], indices.size(), GL_TRIANGLES);
        ibos.push_back(ibo);
        sizeOffset += polygonPointsCount * 8 * sizeof(float);
        countOffset += polygonPointsCount;
        delete[] vertices;
    }

    // Data layout
    VertexBufferLayout layout;
    layout.Push<float>(2);	// x, y
    layout.Push<float>(3);	// Fill   color: r, g, b
    layout.Push<float>(3);	// Border color: r, g, b
    vao->addBuffer(*vbo, layout);

    return featureDesc;
}
