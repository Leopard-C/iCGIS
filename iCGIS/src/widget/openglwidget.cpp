#include "opengl/vertexbufferlayout.h"
#include "widget/openglwidget.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mapbox/earcut.hpp>

#include "geo/geometry/geogeometry.h"
#include "geo/utility/geo_utility.h"
#include "utility.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

double triTime = 0.0;


/*************************************/
/*                                   */
/*         OpenGLWidget              */
/*                                   */
/*************************************/

OpenGLWidget::OpenGLWidget(GeoMap* mapIn, QWidget* parent /*= 0*/)
		: QOpenGLWidget(parent), map(mapIn)
{
	renderer = new Renderer();
}

OpenGLWidget::~OpenGLWidget()
{
	// very important
	makeCurrent();
	isRunning = false;

	if (openglLayerManager)
		delete openglLayerManager;
	if (renderer)
		delete renderer;
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

	// 图层管理
	openglLayerManager = new OpenglLayerManager();
	openglLayerManager->createShaders();
	openglLayerManager->setMap(this->map);
	openglLayerManager->setRenderer(this->renderer);

	// view矩阵
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

	// proj
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
	renderer->Clear();

	if (!isRunning || !map || map->isEmpty())
		return;

	// 绘制地图
	openglLayerManager->drawAll();

	// 是否在拉框选择
	//  动态绘制矩形
	if (isRectSelecting) {
		GLCall(glUseProgram(0));
		drawRectNoFill(mouseBeginPos, mouseCurrPos, 1.0f, 0.5f, 0.0f, 5);
		drawRectFillColor(mouseBeginPos, mouseCurrPos, 1.0f, 0.5f, 0.0f, 0.5f);
	}
}


/********************************/
/*         Unility Function     */
/********************************/

// 世界坐标系坐标到屏幕坐标(在vertex shader中完成）
// 根据MVP矩阵计算得到
// 只是偶尔调用，绘图地图时由GPU进行转换
QPoint OpenGLWidget::xy2screen(double geoX, double geoY)
{
	// 坐标点在规范立方体中的坐标
	// 即 x:[-1, 1]  y:[-1, 1]  z:[-1, 1]
	float stdX = (2 * geoX + 2 * xOffset - (adjustedMapExtent.minX + adjustedMapExtent.maxX)) / adjustedMapExtent.width();
	float stdY = (2 * geoY + 2 * yOffset - (adjustedMapExtent.minY + adjustedMapExtent.maxY)) / adjustedMapExtent.height();

	float screenX = (stdX + 1) * this->width() / 2.0f;
	float screenY = this->height() - (stdY + 1) * this->height() / 2.0f;

	return { int(screenX), int(screenY) };
}


// 屏幕坐标到世界坐标系坐标
GeoRawPoint OpenGLWidget::screen2xy(int screenX, int screenY)
{
	// 鼠标点在规范立方体中的坐标
	// 即 x:[-1, 1]  y:[-1, 1]  z:[-1, 1]
	GeoRawPoint stdXY = screen2stdxy(screenX, screenY);

	float geoX = (stdXY.x * adjustedMapExtent.width() + adjustedMapExtent.minX + adjustedMapExtent.maxX - 2 * xOffset) / (2 * zoom);
	float geoY = (stdXY.y * adjustedMapExtent.height() + adjustedMapExtent.minY + adjustedMapExtent.maxY - 2 * yOffset) / (2 * zoom);

	return { geoX, geoY };
}


// 屏幕坐标到NDC
GeoRawPoint OpenGLWidget::screen2stdxy(int screenX, int screenY)
{
	float stdX = (2.0f * screenX / this->width()) - 1;
	float stdY = (2.0f * (this->height() - screenY) / this->height()) - 1;
	return { stdX, stdY };
}

// 重新计算MVP
void OpenGLWidget::updateMVP(bool updateModel /*= true*/, bool updateView /*= true*/, bool updateProj /*= false*/)
{
	GeoExtent mapExtent = map->getExtent();
	adjustedMapExtent = mapExtent;

	// model矩阵
	if (updateModel) {
		// model
		// 平移
		xOffset = 0.0f;
		yOffset = 0.0f;
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
		// 缩放
		zoom = 1.0f;
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
		model = trans * scale;
	}

	// proj矩阵
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

	// view矩阵
	if (updateView) {
		glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
}

// 设置Mode View Project矩阵
void OpenGLWidget::setMVP()
{
	glm::mat4 mvp = proj * view * model;
	openglLayerManager->setMVP(mvp);
}

/**************************************************/
/*                                                */
/*              Mouse Event                       */
/*                                                */
/**************************************************/

/***************************************/
/*            Mouse Press              */
/***************************************/
void OpenGLWidget::mousePressEvent(QMouseEvent* ev)
{
	mouseBeginPos = mouseLastPos = ev->pos();

	// 清空已经选中的要素
	clearSelected();
}

/***************************************/
/*            Mouse Move               */
/***************************************/
void OpenGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
	mouseCurrPos = ev->pos();

	// 是否已经加载地图
	if (!map || map->isEmpty())
		return;

	GeoExtent mapExtent = map->getExtent();

	makeCurrent();

	// 是否处于编辑状态
	// 编辑状态不可拖动地图
	if (isEditing) {
		// 是否达到框选的最低限度
		if ((mouseCurrPos - mouseBeginPos).manhattanLength() < 6)
			return;
		isRectSelecting = true;
		update();
	}

	// 仅仅是移动图层
	else {
		GeoRawPoint stdLastPos = screen2stdxy(mouseLastPos.x(), mouseLastPos.y());
		GeoRawPoint stdCurrPos = screen2stdxy(mouseCurrPos.x(), mouseCurrPos.y());
		xOffset += ((stdCurrPos.x - stdLastPos.x) * adjustedMapExtent.width()) / 2.0;
		yOffset += ((stdCurrPos.y - stdLastPos.y) * adjustedMapExtent.height()) / 2.0;

		// model
		// 平移
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

	// 是否在编辑状态
	if (isEditing) {
		// 用户是在点选
		if ((mouseCurrPos - mouseBeginPos).manhattanLength() < 6) {
			GeoRawPoint geoXY = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
			std::cout << geoXY.x << " " << geoXY.y << std::endl;
			int layersCount = map->getNumLayers();
			// 按图层优先顺序查询，如果在优先级高的图层查询到，就终止查询
			for (int i = 0; i < layersCount; ++i) {
				GeoLayer* layer = map->getLayerByOrder(i);
				if (layer->getLayerType() == kFeatureLayer) {
					GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
					if (!featureLayer->isEditable() || !featureLayer->isVisable())
						continue;
					GeoFeature* feature = nullptr;
					featureLayer->queryFeatures(geoXY.x, geoXY.y, feature);
					if (feature) {
						openglLayerManager->setSelectedFeature(featureLayer->getLID(), feature);
						update();
						return;
					}
				}
			}
		}
		// 用户是在框选
		else {
			GeoRawPoint leftTop = screen2xy(mouseBeginPos.x(), mouseBeginPos.y());
			GeoRawPoint rightBottom = screen2xy(mouseCurrPos.x(), mouseCurrPos.y());
			GeoExtent rect(leftTop, rightBottom);
			int layersCount = map->getNumLayers();
			// 按图层优先顺序查询，如果在优先级高的图层查询到，就终止查询
			for (int i = 0; i < layersCount; ++i) {
				GeoLayer* layer = map->getLayerByOrder(i);
				if (layer->getLayerType() == kFeatureLayer) {
					GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
					if (!featureLayer->isEditable() || !featureLayer->isVisable())
						continue;
					std::vector<GeoFeature*> features;
					featureLayer->queryFeatures(rect, features);
					if (features.size() > 0) {
						openglLayerManager->setSelectedFeatures(featureLayer->getLID(), features);
						update();
						return;
					}
				}
			}
		}

		update();
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

	// 公式
	//  Delta(xOffset) = (zoomOld - zoomNew) * zoomCenterX;
	//  Delta(yOffset) = (zoomOld - zoomNew) * zoomCenterY;

	// 放大
	if (ev->delta() > 0) {
		xOffset += -0.1 * zoom * geoXY.x;
		yOffset += -0.1 * zoom * geoXY.y;
		zoom *= 1.1;

		// model
		trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
		model = trans * scale;
	}
	// 缩小
	else {
		xOffset += 0.1 / 1.1 * zoom * geoXY.x;
		yOffset += 0.1 / 1.1 * zoom * geoXY.y;
		zoom /= 1.1;

		// model
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

void OpenGLWidget::onLayerOrderChanged()
{
	openglLayerManager->updateLayersOrder();
	update();
}

void OpenGLWidget::onStartEditing(bool editiable)
{
	isEditing = editiable;
	update();
}

void OpenGLWidget::onRemoveLayer(int nLID)
{
	openglLayerManager->removeLayer(nLID);
	update();
}

void OpenGLWidget::onSelectFeature(int nLID, int nFID)
{
	openglLayerManager->clearSelected();
	openglLayerManager->setSelectedFeature(nLID, nFID);
	update();
}

void OpenGLWidget::onSelectFeature(int nLID, GeoFeature* feature)
{
	openglLayerManager->clearSelected();
	openglLayerManager->setSelectedFeature(nLID, feature);
	update();
}

void OpenGLWidget::onSelectFeatures(int nLID, const std::vector<int>& nFIDs)
{
	openglLayerManager->clearSelected();
	openglLayerManager->setSelectedFeatures(nLID, nFIDs);
	update();
}

void OpenGLWidget::onSelectFeatures(int nLID, const std::vector<GeoFeature*>& features)
{
	openglLayerManager->clearSelected();
	openglLayerManager->setSelectedFeatures(nLID, features);
	update();
}

/* 找出地图中未发送给GPU的图层，并发送给GPU */
void OpenGLWidget::onAutoSendDataToGPU()
{
	int layersCount = map->getNumLayers();
	for (int i = 0; i < layersCount; ++i) {
		GeoLayer* layer = map->getLayerById(i);
		if (!openglLayerManager->hasSendToGPU(layer->getLID())) {
			if (layer->getLayerType() == kFeatureLayer)
				sendDataToGPU(layer->toFeatureLayer());
			else
				sendDataToGPU(layer->toRasterLayer());
		}
	}
}

/* 缩放至图层 */
void OpenGLWidget::onZoomToLayer(int nLID)
{
	GeoLayer* layer = map->getLayerByLID(nLID);
	if (!layer)
		return;
	GeoExtent layerExtent = layer->getExtent();

	if (layerExtent.aspectRatio() < adjustedMapExtent.aspectRatio())
		zoom = adjustedMapExtent.height() / layerExtent.height();
	else
		zoom = adjustedMapExtent.width() / layerExtent.width();

	xOffset = adjustedMapExtent.centerX() - layerExtent.centerX();
	yOffset = adjustedMapExtent.centerY() - layerExtent.centerY();
	xOffset += (1.0f - zoom) * layerExtent.centerX();
	yOffset += (1.0f - zoom) * layerExtent.centerY();

	// model
	trans = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, yOffset, 0.0f));
	scale = glm::scale(glm::mat4(1.0f), glm::vec3(zoom, zoom, 1.0f));
	model = trans * scale;

	setMVP();
	update();
}


/**************************************************/
/*                                                */
/*            private functions                   */
/*                                                */
/**************************************************/

// 清空选中的要素
void OpenGLWidget::clearSelected()
{
	// get opengl contex
	makeCurrent();

	openglLayerManager->clearSelected();

	update();
}

// 绘制填充矩形
void OpenGLWidget::drawRectFillColor(const QPoint& startPoint, const QPoint& endPoint, float r, float g, float b, float a)
{
	GeoRawPoint leftTop = screen2stdxy(startPoint.x(), startPoint.y());
	GeoRawPoint rightBottom = screen2stdxy(endPoint.x(), endPoint.y());

	// 如果设置了透明度，就启用混合
	bool usingBlend = fabs(a - 1.0f) < 0.001f ? false : true;
	if (usingBlend) {
		glEnable(GL_BLEND);	// 启用混合
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// 混合函数
	}

	glColor4f(r, g, b, a);
	glRectd(leftTop.x, leftTop.y, rightBottom.x, rightBottom.y);	// 绘制矩形内部

	if (usingBlend) {
		glDisable(GL_BLEND);// 关闭混合
	}
}

// 绘制矩形边框
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

/**************************************************/
/*                                                */
/*            public functions                    */
/*                                                */
/**************************************************/

/* 修改要素的填充色 */
void OpenGLWidget::setFillColor(int nLID, int nFID, int r, int g, int b, bool bUpdate/* = true*/)
{
	makeCurrent();

	openglLayerManager->setFeatureFillColor(nLID, nFID, r, g, b);

	// 是否立即刷新
	if (bUpdate)
		update();
}


/*********************************************/
/*                                           */
/*        SEND raster data to GPU            */
/*                                           */
/*********************************************/

void OpenGLWidget::sendDataToGPU(GeoRasterLayer* rasterLayer)
{
	if (!rasterLayer)
		return;
	int nLID = rasterLayer->getLID();

	makeCurrent();

	// 栅格图层描述符
	OpenglRasterLayerDescriptor* rasterLayerDesc = new OpenglRasterLayerDescriptor(nLID);
	if (!openglLayerManager->addLayerDescriptor(rasterLayerDesc)) {
		delete rasterLayerDesc;
		return;
	}

	GeoExtent extent = rasterLayer->getExtent();

	// 栅格数据描述符
	OpenglRasterDescriptor* rasterDesc = new OpenglRasterDescriptor();
	rasterLayerDesc->rasterDesc = rasterDesc;
	
	// VAO
	VertexArray* vao = new VertexArray();
	rasterDesc->setVAO(vao);

	// VBO
	float vertices[] = {
		// position					// texture coords
		extent.maxX, extent.minY,   1.0f, 1.0f,
		extent.maxX, extent.maxY,   1.0f, 0.0f,
		extent.minX, extent.maxY,   0.0f, 0.0f,
		extent.minX, extent.minY,   0.0f, 1.0f
	};
	VertexBuffer* vbo = new VertexBuffer(vertices, 16 * sizeof(float));
	rasterDesc->setVBO(vbo);

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(2);	// coordX, coordY
	vao->addBuffer(*vbo, layout);

	// IBO
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	IndexBuffer* ibo = new IndexBuffer(indices, 6, GL_TRIANGLES);
	rasterDesc->setIBO(ibo);

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
			// 像素值规范化为 [0, 1]
			float maxValue = *(std::max_element((float*)(band->pData), (float*)(band->pData) + count));
			float* pixels = (float*)(band->pData);
			float* buff = new float[count];
			for (int i = 0; i < count; ++i) {
				buff[i] = pixels[i] / maxValue;
			}
			texture = new Texture(buff, band->width, band->height,
				GL_FLOAT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
			delete[] buff;
			rasterDesc->addTex(texture);
			break;
		}
		}
	}

	/**** 设置MVP矩阵 ***/
	if (map->getNumLayers() == 1)
		updateMVP(true, false, true);
	else
		updateMVP(false, false, true);

	setMVP();

	update();
}


/*********************************************/
/*                                           */
/*        SEND vector data to GPU            */
/*                                           */
/*********************************************/

void OpenGLWidget::sendDataToGPU(GeoFeatureLayer* featureLayer)
{
	if (!featureLayer)
		return;
	int nLID = featureLayer->getLID();

	makeCurrent();

	clock_t start = clock();

	GeoExtent extent = featureLayer->getExtent();

	// 要素图层描述符
	OpenglFeatureLayerDescriptor* featureLayerDesc = new OpenglFeatureLayerDescriptor(nLID);
	if (!openglLayerManager->addLayerDescriptor(featureLayerDesc)) {
		delete featureLayerDesc;
		return;
	}

	// 随机填色
	float r = utils::getRand(0.3f, 0.6f);
	float g = utils::getRand(0.3f, 0.6f);
	float b = utils::getRand(0.2f, 0.5f);

	for (auto layerIt = featureLayer->begin(); layerIt != featureLayer->end(); ++layerIt) {
		const auto& feature = *layerIt;
		int nFID = feature->getFID();
		GeometryType geomType = feature->getGeometryType();

		// feature描述符
		OpenglFeatureDescriptor* featureDesc = new OpenglFeatureDescriptor(nFID, geomType);
		featureLayerDesc->addFeatureDesc(featureDesc);

		switch (geomType) {
		default:
			break;
		case kPoint:
		{
			GeoPoint* point = feature->getGeometry()->toPoint();
			sendPointToGPU(point, r, g, b, featureDesc);
			break;
		}
		case kPolygon:
		{
			GeoPolygon* polygon = feature->getGeometry()->toPolygon();
			sendPolygonToGPU(polygon, r, g, b, featureDesc);
			break;
		}
		case kLineString:
		{
			GeoLineString* lineString = feature->getGeometry()->toLineString();
			sendLineStringToGPU(lineString, 0.0f, 0.0f, 0.0f, featureDesc);
			break;
		}
		case kMultiPoint:
		{
			GeoMultiPoint* multiPoint = feature->getGeometry()->toMultiPoint();
			sendMultiPointToGPU(multiPoint, 0.0f, 0.0f, 0.0f, featureDesc);
			break;
		}
		case kMultiPolygon:
		{
			GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
			sendMultiPolygonToGPU(multiPolygon, r, g, b, featureDesc);
			break;
		}
		case kMultiLineString:
		{
			GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
			sendMultiLineStringToGPU(multiLineString, r, g, b, featureDesc);
			break;
		}
		}
	}

	clock_t end = clock();

	std::cout << "Triangulation time:" << triTime / 1e6 << "s" << std::endl;
	std::cout << "Send data to gpu time:" << (end - start) / double(CLK_TCK) << "s" << std::endl;

	/**** 设置MVP矩阵 ***/
	if (map->getNumLayers() == 1)
		updateMVP(true, false, true);
	else
		updateMVP(false, false, true);

	setMVP();

	update();
}


void OpenGLWidget::sendPointToGPU(GeoPoint* point, float r, float g, float b, OpenglFeatureDescriptor*& featureDesc)
{
	// VAO
	VertexArray* vao = new VertexArray();
	featureDesc->setVAO(vao);

	// VBO
	float vertices[5] = { point->getX(), point->getY(), r, g, b };
	VertexBuffer* vbo = new VertexBuffer(vertices, 5 * sizeof(float));
	featureDesc->setVBO(vbo);

	// IBO
	unsigned int indices = 0;
	IndexBuffer* ibo = new IndexBuffer(&indices, 1, GL_POINTS);
	featureDesc->addIBO(ibo);

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}

void OpenGLWidget::sendMultiPointToGPU(GeoMultiPoint* mutliPoint, float r, float g, float b, OpenglFeatureDescriptor*& featureDesc)
{
	int pointsCount = mutliPoint->getNumGeometries();

	// VAO
	VertexArray* vao = new VertexArray();
	featureDesc->setVAO(vao);

	// VBO
	float* vertices = new float[pointsCount * 5 * sizeof(float)];
	int index = 0;
	GeoPoint* point;
	for (int i = 0; i < pointsCount; ++i) {
		point = mutliPoint->getGeometry(i)->toPoint();
		vertices[i * 5] = point->getX();
		vertices[i * 5 + 1] = point->getY();
		vertices[i * 5 + 2] = r;
		vertices[i * 5 + 3] = g;
		vertices[i * 5 + 4] = b;
	}
	VertexBuffer* vbo = new VertexBuffer(vertices, pointsCount * 5 * sizeof(float));
	featureDesc->setVBO(vbo);

	// IBO
	unsigned int* indices = utils::newContinuousNumber(0, pointsCount);
	IndexBuffer* ibo = new IndexBuffer(indices, pointsCount, GL_POINTS);
	delete[] vertices;
	delete[] indices;

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}

void OpenGLWidget::sendLineStringToGPU(GeoLineString* lineString, float r, float g, float b, OpenglFeatureDescriptor*& featureDesc)
{
	int pointsCount = lineString->getNumPoints();

	// VAO
	VertexArray* vao = new VertexArray();
	featureDesc->setVAO(vao);
	
	// VBO
	float* vertices = new float[pointsCount * 5];
	for (int i = 0; i < pointsCount; ++i) {
		vertices[i * 5] = lineString->getX(i);
		vertices[i * 5 + 1] = lineString->getY(i);
		vertices[i * 5 + 2] = r;
		vertices[i * 5 + 3] = g;
		vertices[i * 5 + 4] = b;
	}
	VertexBuffer* vbo = new VertexBuffer(vertices, 5 * sizeof(float) * pointsCount);
	featureDesc->setVBO(vbo);

	// IBO
	unsigned int* indices = utils::newContinuousNumber(0, pointsCount);
	IndexBuffer* ibo = new IndexBuffer(indices, pointsCount, GL_LINE_STRIP);
	featureDesc->addIBO(ibo);
	delete[] vertices;
	delete[] indices;

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}

void OpenGLWidget::sendMultiLineStringToGPU(GeoMultiLineString* multiLineString, float r, float g, float b, OpenglFeatureDescriptor*& featureDesc)
{
	int pointsCount = multiLineString->getNumPoints();
	int linesCount = multiLineString->getNumGeometries();

	// VAO
	VertexArray* vao = new VertexArray();
	featureDesc->setVAO(vao);
	
	// VBO
	VertexBuffer* vbo = new VertexBuffer(nullptr, 5 * sizeof(float) * pointsCount);
	featureDesc->setVBO(vbo);

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
		featureDesc->addIBO(ibo);
		sizeOffset += pointsCount * 5 * sizeof(float);
		countOffset += pointsCount;
		delete[] vertices;
		delete[] indices;
	}

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}

void OpenGLWidget::sendPolygonToGPU(GeoPolygon* geoPolygon, float r,float g, float b,
	OpenglFeatureDescriptor*& featureDesc)
{
	int polygonPointsCount = geoPolygon->getNumPoints();
	int interiorRingsCount = geoPolygon->getInteriorRingsCount();

	// VAO
	VertexArray* vao = new VertexArray();
	vao->reserve(interiorRingsCount + 1);
	featureDesc->setVAO(vao);

	float* vertices = new float[polygonPointsCount * 5 * sizeof(float)];
	int iStride = 0;

	using Point = std::array<double, 2>;
	std::vector<std::vector<Point>> polygon;

	// 外环
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
		vertices[index] = rawPoint.x;
		vertices[index + 1] = rawPoint.y;
		vertices[index + 2] = r;
		vertices[index + 3] = g;
		vertices[index + 4] = b;
		index += 5;
	}
	polygon.emplace_back(exteriorRing);

	// 内环
	for (int j = 0; j < interiorRingsCount; ++j) {
		const auto& geoInteriorRing = geoPolygon->getInteriorRing(j);
		int interiorRingPointsCount = geoInteriorRing->getNumPoints();
		vao->setStride(iStride++, interiorRingPointsCount);
		std::vector<Point> interiorRing;
		interiorRing.reserve(interiorRingPointsCount);
		for (int k = 0; k < interiorRingPointsCount; ++k) {
			geoInteriorRing->getRawPoint(k, &rawPoint);
			exteriorRing.push_back({ rawPoint.x, rawPoint.y });
			vertices[index] = rawPoint.x;
			vertices[index + 1] = rawPoint.y;
			vertices[index + 2] = r;
			vertices[index + 3] = g;
			vertices[index + 4] = b;
			index += 5;
		}
		polygon.emplace_back(interiorRing);
	}

	auto start = std::chrono::system_clock::now();

	// 三角剖分
	std::vector<unsigned int> indices = mapbox::earcut<unsigned int>(polygon);
	//std::reverse(indices.begin(), indices.end());

	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	triTime += double(duration.count());

	// VBO
	VertexBuffer* vbo = new VertexBuffer(vertices, polygonPointsCount * 5 * sizeof(float));
	featureDesc->setVBO(vbo);

	// IBO
	IndexBuffer* ibo = new IndexBuffer(&indices[0], indices.size(), GL_TRIANGLES);
	featureDesc->addIBO(ibo);
	delete[] vertices;

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}


void OpenGLWidget::sendMultiPolygonToGPU(GeoMultiPolygon* multiPolygon, float r,float g, float b, 
	OpenglFeatureDescriptor*& featureDesc)
{
	int pointsCount = multiPolygon->getNumPoints();
	int polygonCount = multiPolygon->getNumGeometries();
	int linearRingsCount = multiPolygon->getNumLinearRings();

	// VAO
	VertexArray* vao = new VertexArray();
	vao->reserve(linearRingsCount);
	featureDesc->setVAO(vao);

	int sizeOffset = 0;
	int countOffset = 0;
	int subDataSize = 0;

	// VBO，这里只是分配空间，没有发送数据
	VertexBuffer* vbo = new VertexBuffer(nullptr, pointsCount * 5 * sizeof(float));
	featureDesc->setVBO(vbo);
	int iStride = 0;

	for (int i = 0; i < polygonCount; ++i) {
		GeoPolygon* geoPolygon = multiPolygon->getPolygon(i);
		int polygonPointsCount = geoPolygon->getNumPoints();
		int interiorRingsCount = geoPolygon->getInteriorRingsCount();

		float* vertices = new float[polygonPointsCount * 5 * sizeof(float)];

		using Point = std::array<double, 2>;
		std::vector<std::vector<Point>> polygon;

		// 外环
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
			vertices[index] = rawPoint.x;
			vertices[index + 1] = rawPoint.y;
			vertices[index + 2] = r;
			vertices[index + 3] = g;
			vertices[index + 4] = b;
			index += 5;
		}
		polygon.emplace_back(exteriorRing);

		// 内环
		for (int j = 0; j < interiorRingsCount; ++j) {
			const auto& geoInteriorRing = geoPolygon->getInteriorRing(j);
			int interiorRingPointsCount = geoInteriorRing->getNumPoints();
			vao->setStride(iStride++, interiorRingPointsCount);
			std::vector<Point> interiorRing;
			interiorRing.reserve(interiorRingPointsCount);
			for (int k = 0; k < interiorRingPointsCount; ++k) {
				geoInteriorRing->getRawPoint(k, &rawPoint);
				exteriorRing.push_back({ rawPoint.x, rawPoint.y });
				vertices[index] = rawPoint.x;
				vertices[index + 1] = rawPoint.y;
				vertices[index + 2] = r;
				vertices[index + 3] = g;
				vertices[index + 4] = b;
				index += 5;
			}
			polygon.emplace_back(interiorRing);
		}

		auto start = std::chrono::system_clock::now();

		// 三角剖分
		std::vector<unsigned int> indices = mapbox::earcut<unsigned int>(polygon);
		//std::reverse(indices.begin(), indices.end());

		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		triTime += double(duration.count());

		if (countOffset > 0)
			std::for_each(indices.begin(), indices.end(), [&countOffset](auto& value) {value += countOffset; });

		// VBO
		vbo->addSubData(vertices, sizeOffset, polygonPointsCount * 5 * sizeof(float));

		// IBO
		IndexBuffer* ibo = new IndexBuffer(&indices[0], indices.size(), GL_TRIANGLES);
		featureDesc->addIBO(ibo);
		sizeOffset += polygonPointsCount * 5 * sizeof(float);
		countOffset += polygonPointsCount;
		delete[] vertices;
	}

	// 告诉OpenGL数据是如何布局的
	VertexBufferLayout layout;
	layout.Push<float>(2);	// x, y
	layout.Push<float>(3);	// r, g, b
	vao->addBuffer(*vbo, layout);
}


