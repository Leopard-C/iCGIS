/************************************************************************************
** class name:  OpenglLayerManager
**
** description: 在OpenGL中的图层管理类，管理每个图层的描述符(OpenglFeatureLayerDescriptor)
**				绘制地图、选中要素等是由该类处理
**
** last change: 2020-01-02
************************************************************************************/
#pragma once

#include "opengl/layermanager/opengllayerdescriptor.h"
#include "opengl/shader.h"
#include "opengl/renderer.h"
#include "geo/map/geomap.h"


class OpenglLayerManager {
public:
	OpenglLayerManager();
	~OpenglLayerManager();

	void setMap(GeoMap* mapIn) { map = mapIn; }
	void setRenderer(Renderer* rendererIn) { renderer = rendererIn; }

	// 创建着色器： 点、线、面、边界、高亮
	void createShaders();

	// 移除指定图层描述符
	void removeLayer(int nLID);

	// 更新图层顺序
	void updateLayersOrder();

	// 添加图层描述符
	bool addLayerDescriptor(OpenglLayerDescriptor* descriptor);

	// 绘制所有可见图层
	void drawAll();

	// 绘制要素图层
	void drawFeatureLayers(OpenglFeatureLayerDescriptor* featureLayerDesc);;
	// 绘制栅格图层
	void drawRasterLayers(OpenglRasterLayerDescriptor* rasterLayerDesc);
	// 绘制被选中的(要素)图层
	void drawSelectedFeatures();

	// 绘制几何图形
	void drawPoints(OpenglFeatureDescriptor* featureDesc, Shader* shader);
	void drawMultiPoints(OpenglFeatureDescriptor* featureDesc, Shader* shader);
	void drawPolygon(OpenglFeatureDescriptor* featureDesc, Shader* fillShader, Shader* borderShader);
	void drawMultiPolygon(OpenglFeatureDescriptor* featureDesc, Shader* fillShader, Shader* borderShader);
	void drawLineString(OpenglFeatureDescriptor* featureDesc, Shader* shader);
	void drawMultiLineString(OpenglFeatureDescriptor* featureDesc, Shader* shader);

	// 设置要素填充颜色
	void setFeatureFillColor(int nLID, int nFID, int r, int g, int b);

	// 设置MVP
	void setMVP(const glm::mat4& mvp);

	// 设置要素的选中与取消
	void setSelectedFeature(int nLID, int nFID);
	void setSelectedFeature(int nLID, GeoFeature* feature);
	void setSelectedFeatures(int nLID, const std::vector<int>& nFIDs);
	void setSelectedFeatures(int nLID, const std::vector<GeoFeature*>& features);
	void clearSelected();

	// 图层是否已经发送给GPU
	bool hasSendToGPU(int nLID);

private:
	OpenglLayerDescriptor* getLayerDescriptor(int nLID) const;

private:
	std::vector<OpenglLayerDescriptor*> layerDescriptors;

	// 临时存放被选中的feature
	// 最后高亮绘制它们（如果不最后绘制，会被后续的遮挡）
	std::vector<OpenglFeatureDescriptor*> selectedFeatures;
	int selectedLID;
	
	// shaders
	Shader* pointShader = nullptr;
	Shader* lineShader = nullptr;
	Shader* polygonShader = nullptr;
	Shader* borderShader = nullptr;
	Shader* highlightShader = nullptr;
	Shader* textureShader = nullptr;

	// renderer
	Renderer* renderer = nullptr;

	// geomap
	GeoMap* map = nullptr;
};
