/**************************************************************************
** class name:  OpenGLWidget
**
** description: OpenGL widget, public inherited from QOpengGLWidget
**
** last change: 2020-03-27
**************************************************************************/
#pragma once

#include "opengl/glcall.h"
#include "util/memoryleakdetect.h"
#include "geo/map/geomap.h"
#include "opengl/renderer.h"

#include "dialog/whatisthisdialog.h"
#include "operation/operationlist.h"

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <vector>

// OpenGL math library
#include <glm/glm.hpp>



class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget* parent = nullptr);
    ~OpenGLWidget();

signals:
    // Status bar
    // Change the cursor position at the world system timely in real time.
    void sigUpdateCoord(double geoX, double geoY);

public slots:
    // Send Data to GPU
    void onSendMapToGPU(bool bUpdate = true);
    void onSendLayerToGPU(GeoLayer* layer, bool bUpdate = true);
    void onSendFeatureToGPU(GeoFeature* feature);
    void onSendFeatureLayerToGPU(GeoFeatureLayer* featureLayer, bool bUpdate = true);
    void onSendRasterLayerToGPU(GeoRasterLayer* rasterLayer, bool bUpdate = true);

    //void onZoomToLayer(int nLID);
    void onZoomToLayer(GeoLayer* layer);
    void onZoomToMap();

private:
    GeoMap*& map;

    // WCS ==> SCS  (world/screen coordinate system)
    // Just use occasionally
    // When draw shapes, it will be processed by GPU
    QPoint xy2screen(double geoX, double geoY);

    // SCS ==> WCS
    GeoRawPoint screen2xy(int screenX, int screenY);

    // SCS ==> [-1.0 <= x,y,x <= 1.0f]
    GeoRawPoint screen2stdxy(int screenX, int screenY);

    // Length in: SCS ==> WCS
    double getLengthInWorldSystem(int screenLength);

    // Update MVP matrix
    void updateMVP(bool updateModel = true, bool updateView = true, bool updateProj = false);
	void setMVP();

    // clear selected features
    void clearSelected();

    // Draw rectangle width fill color
    void drawRectFillColor(const QPoint& startPoint, const QPoint& endPoint,
                           float r, float g, float b, float a = 1.0f);

    // Draw rectangel with border only
    void drawRectNoFill(const QPoint& startPoint, const QPoint& endPoint,
                        float r = 0.0f, float g = 0.0f, float b = 0.0f, int lineWidth = 1);

protected:
    /* override */
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    /* Event */
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) override;
    virtual void wheelEvent(QWheelEvent* ev) override;
    virtual void enterEvent(QEvent*) override;
    virtual void keyPressEvent(QKeyEvent* ev) override;

private:
    OpenglFeatureDescriptor* sendPointToGPU(GeoPoint* point, float r, float g, float b);
    OpenglFeatureDescriptor* sendMultiPointToGPU(GeoMultiPoint* mutliPoint, float r, float g, float b);
    OpenglFeatureDescriptor* sendLineStringToGPU(GeoLineString* lineString, float r, float g, float b);
    OpenglFeatureDescriptor* sendMultiLineStringToGPU(GeoMultiLineString* multiLineString, float r, float g, float b);
    OpenglFeatureDescriptor* sendPolygonToGPU(GeoPolygon* polygon, float r, float g, float b);
    OpenglFeatureDescriptor* sendMultiPolygonToGPU(GeoMultiPolygon* multiPolygon, float r, float g, float b);

private:
    /************************* MVP **************************/
    // model
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    float zoom   = 1.0f;
    glm::mat4 trans;	// translation
    glm::mat4 scale;	// sacle
    glm::mat4 model;
    // view
    glm::mat4 view;
    // project
    glm::mat4 proj;
    /********************** End MVP *************************/

    // Adjust map's extent width the same aspect ration of OpenGL widget
    GeoExtent adjustedMapExtent;

    bool isMouseClicked = false;
    bool isRunning = true;
    bool isRectSelecting = false;
    bool isSelected = false;
    bool isMovingFeatures = false;
    bool isModified = false;

    // Record the operations: move features, delete features, etc.
    OperationList& opList;

    QPoint mouseLastPos;
    QPoint mouseBeginPos;   // when press left button
    QPoint mouseCurrPos;    // when move mouse

    // What is this
    WhatIsThisDialog* whatIsThisDialog = nullptr;
};
