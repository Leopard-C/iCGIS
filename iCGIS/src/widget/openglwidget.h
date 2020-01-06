/*******************************************************
** class name:  OpenGLWidget
**
** description: OpenGL控件，用于绘制地图
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "memoryleakdetect.h"
#include "geo/map/geomap.h"
#include "opengl/renderer.h"
#include "opengl/layermanager/opengllayermanager.h"

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <vector>

// OpenGL数学库
#include <glm/glm.hpp>


class OpenGLWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit OpenGLWidget(GeoMap* mapIn, QWidget* parent = 0);
	~OpenGLWidget();

signals:	

public:
	// 发送FeatureLayer数据到GPU
	void sendDataToGPU(GeoFeatureLayer* featureLayer);
	// 发送RasterLayer数据到GPU
	void sendDataToGPU(GeoRasterLayer* rasterLayer);

public slots:
	void onLayerOrderChanged();
	void onStartEditing(bool);
	void onRemoveLayer(int nLID);
	void onZoomToLayer(int nLID);
	void onSelectFeature(int nLID, int nFID);
	void onSelectFeature(int nLID, GeoFeature* feature);
	void onSelectFeatures(int nLID, const std::vector<int>& nFIDs);
	void onSelectFeatures(int nLID, const std::vector<GeoFeature*>& features);

	void onAutoSendDataToGPU();

	// 修改要素的填充色
	void setFillColor(int nLID, int nFID, int r, int g, int b, bool bUpdate = true);



private:
	// 世界坐标系坐标到屏幕坐标
	// 偶尔使用，绘图时，由GPU进行计算转换
	QPoint xy2screen(double geoX, double geoY);

	// 屏幕坐标到世界坐标系坐标
	GeoRawPoint screen2xy(int screenX, int screenY);

	// 屏幕坐标到规范立方体坐标(-1.0f <= x,y,z <= 1.0f)
	GeoRawPoint screen2stdxy(int screenX, int screenY);

	// 修改MVP矩阵
	void updateMVP(bool updateModel = true, bool updateView = true, bool updateProj = false);	// 重新计算MVP
	void setMVP();		// 将新的MVP传递给OpenGL

	// 清空选中的要素
	void clearSelected();

	// 绘制填充矩形
	void drawRectFillColor(const QPoint& startPoint, const QPoint& endPoint,
		float r, float g, float b, float a = 1.0f);

	// 绘制矩形边框
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

private:
	void sendPointToGPU(GeoPoint* point, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);
	void sendMultiPointToGPU(GeoMultiPoint* mutliPoint, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);
	void sendLineStringToGPU(GeoLineString* lineString, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);
	void sendMultiLineStringToGPU(GeoMultiLineString* multiLineString, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);
	void sendPolygonToGPU(GeoPolygon* polygon, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);
	void sendMultiPolygonToGPU(GeoMultiPolygon* multiPolygon, float r, float g, float b,
		OpenglFeatureDescriptor*& featureDesc);

public:
	OpenglLayerManager* openglLayerManager = nullptr;

private:
	/************************* MVP **************************/
	// model
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	float zoom   = 1.0f;
	glm::mat4 trans;	// 平移
	glm::mat4 scale;	// 缩放
	glm::mat4 model;
	// view
	glm::mat4 view;
	// project
	glm::mat4 proj;
	/********************** End MVP *************************/

	// 调整的地图范围（调整至和视口的长宽比一致）
	GeoExtent adjustedMapExtent;

	bool isRunning = true;
	bool isEditing = false;
	bool isRectSelecting = false;

	GeoMap* map = nullptr;
	Renderer* renderer = nullptr;

	// 用于 mouse move 事件 
	QPoint mouseLastPos;
	// 用于 mouse release 事件
	QPoint mouseBeginPos;
	QPoint mouseCurrPos;
};

