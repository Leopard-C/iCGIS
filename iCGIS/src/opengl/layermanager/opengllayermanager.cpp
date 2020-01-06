#include "opengl/layermanager/opengllayermanager.h"
#include "geo/map/geolayer.h"

#include "opengl/glcall.h"

OpenglLayerManager::OpenglLayerManager()
{
}

OpenglLayerManager::~OpenglLayerManager()
{
	for (auto& descriptor : layerDescriptors)
		delete descriptor;

	if (lineShader)
		delete lineShader;
	if (pointShader)
		delete pointShader;
	if (polygonShader)
		delete polygonShader;
	if (borderShader)
		delete borderShader;
	if (highlightShader)
		delete highlightShader;
	if (textureShader)
		delete textureShader;
}


/***************************************************/
/*                                                 */
/*             Public Functions                    */
/*                                                 */
/***************************************************/

// 创建着色器
void OpenglLayerManager::createShaders()
{
	pointShader = new Shader("res/shaders/point_vert.glsl", "res/shaders/point_frag.glsl");
	lineShader = new Shader("res/shaders/line_vert.glsl", "res/shaders/line_frag.glsl");
	polygonShader = new Shader("res/shaders/polygon_vert.glsl", "res/shaders/polygon_frag.glsl");
	borderShader = new Shader("res/shaders/border_vert.glsl", "res/shaders/border_frag.glsl");
	highlightShader = new Shader("res/shaders/highlight_vert.glsl", "res/shaders/highlight_frag.glsl");
	textureShader = new Shader("res/shaders/texture_vert.glsl", "res/shaders/texture_frag.glsl");
	borderShader->Bind();
	borderShader->SetUniform3f("u_borderColor", 0.0f, 0.0f, 0.0f);
	highlightShader->Bind();
	highlightShader->SetUniform3f("u_highlightColor", 0.0f, 1.0f, 1.0f);
}

// 移除图层
void OpenglLayerManager::removeLayer(int nLID)
{
	auto layerDesc = layerDescriptors.begin();
	while (layerDesc != layerDescriptors.end()) {
		if ((*layerDesc)->LID == nLID) {
			delete (*layerDesc);
			layerDescriptors.erase(layerDesc);
			return;
		}
		++layerDesc;
	}
}

/* 更新图层顺序 */
void OpenglLayerManager::updateLayersOrder()
{
	int count = layerDescriptors.size();
	auto tempLayerDescriptors(layerDescriptors);
	for (int iOrder = 0; iOrder < count; ++iOrder) {
		int nLID = map->getLayerLIDByOrder(iOrder);
		for (int j = 0; j < count; ++j) {
			if (tempLayerDescriptors[j]->LID == nLID) {
				layerDescriptors[count - iOrder - 1] = tempLayerDescriptors[j];
				break;;
			}
		}
	}
}

// 添加图层描述符
bool OpenglLayerManager::addLayerDescriptor(OpenglLayerDescriptor* descriptor)
{
	int nLID = descriptor->LID;
	for (const auto& layerDesc : layerDescriptors) {
		if (layerDesc->LID == nLID) {
			return false;
		}
	}
	layerDescriptors.push_back(descriptor);
	return true;
}

// 获取图层描述符
OpenglLayerDescriptor* OpenglLayerManager::getLayerDescriptor(int nLID) const
{
	for (const auto& layerDescriptor : layerDescriptors) {
		if (layerDescriptor->LID == nLID) {
			return layerDescriptor;
		}
	}
	return nullptr;
}

// 设置要素填充颜色
void OpenglLayerManager::setFeatureFillColor(int nLID, int nFID, int r, int g, int b)
{
	OpenglLayerDescriptor* layerDesc = getLayerDescriptor(nLID);
	if (!layerDesc && layerDesc->getLayerType() == kFeatureLayer)
		return;

	auto featureLayerDesc = layerDesc->toFeatureLayerDescriptor();

	OpenglFeatureDescriptor* featureDesc = featureLayerDesc->getFeatureDescriptor(nFID);
	if (!featureDesc)
		return;

	unsigned int count = featureDesc->vbo->getSize() / sizeof(float);

	float* data;
	featureDesc->vbo->Bind();
	GLCall(data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

	// 数据布局 （x, y, r, g, b）
	for (int i = 0; i < count; i += 5) {
		// data[i] = data[i];				// x
		// data[i + 1] = data[i + 1];		// y
		data[i + 2] = r / 255.0f;			// r
		data[i + 3] = g / 255.0f;			// g
		data[i + 4] = b / 255.0f;			// b
	}

	GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}

// 选中要素
void OpenglLayerManager::setSelectedFeature(int nLID, GeoFeature* feature)
{
	setSelectedFeature(nLID, feature->getFID());
}

void OpenglLayerManager::setSelectedFeature(int nLID, int nFID)
{
	selectedLID = nLID;
	//GeoLayer* layer = map->getLayerById(nLID);
	for (auto& layerDesc : layerDescriptors) {
		if (layerDesc->LID == nLID && layerDesc->getLayerType() == kFeatureLayer) {
			auto featureLayerDesc = layerDesc->toFeatureLayerDescriptor();
			for (auto& featureDesc : featureLayerDesc->featuresDescriptors) {
				if (featureDesc->FID == nFID) {
					selectedFeatures.push_back(featureDesc);
					featureDesc->isSelected = true;
					return;
				}
			}
		}
	}
}

void OpenglLayerManager::setSelectedFeatures(int nLID, const std::vector<int>& nFIDs)
{
	for (auto nFID : nFIDs) {
		setSelectedFeature(nLID, nFID);
	}
}

void OpenglLayerManager::setSelectedFeatures(int nLID, const std::vector<GeoFeature*>& features)
{
	for (auto& feature : features) {
		setSelectedFeature(nLID, feature->getFID());
	}
}

// 清空选中的要素
void OpenglLayerManager::clearSelected()
{
	selectedFeatures.clear();
	for (auto& layerDesc : layerDescriptors) {
		if (layerDesc->getLayerType() == kFeatureLayer) {
			auto featureLayerDesc = layerDesc->toFeatureLayerDescriptor();
			for (auto& featureDesc : featureLayerDesc->featuresDescriptors) {
				featureDesc->isSelected = false;
			}
		}
	}
}

// 图层是否已经发送给GPU
bool OpenglLayerManager::hasSendToGPU(int nLID)
{
	for (auto& layerDesc : layerDescriptors) {
		if (layerDesc->LID == nLID) {
			return true;
		}
	}
	return false;
}

// 设置MVP矩阵
void OpenglLayerManager::setMVP(const glm::mat4& mvp)
{
	pointShader->Bind();
	pointShader->SetUniformMat4f("u_MVP", mvp);

	lineShader->Bind();
	lineShader->SetUniformMat4f("u_MVP", mvp);

	polygonShader->Bind();
	polygonShader->SetUniformMat4f("u_MVP", mvp);

	borderShader->Bind();
	borderShader->SetUniformMat4f("u_MVP", mvp);

	highlightShader->Bind();
	highlightShader->SetUniformMat4f("u_MVP", mvp);

	textureShader->Bind();
	textureShader->SetUniformMat4f("u_MVP", mvp);
}

// 绘制地图
void OpenglLayerManager::drawAll()
{
	// 绘制(未被选中的）
	for (const auto& layerDesc : layerDescriptors) {
		int nLID = layerDesc->LID;
		GeoLayer* layer = map->getLayerByLID(nLID);

		// 图层是否可见
		if (!layer ||!layer->isVisable())
			continue;

		// 要素图层
		if (layerDesc->getLayerType() == kFeatureLayer)
			drawFeatureLayers(layerDesc->toFeatureLayerDescriptor());
		// 栅格图层
		else
			drawRasterLayers(layerDesc->toRasterLayerDescriptor());

		if (selectedLID != nLID)
			continue;

		// 该图层中被选中的要素
		drawSelectedFeatures();
	}
}


/* 绘制要素图层 */
void OpenglLayerManager::drawFeatureLayers(OpenglFeatureLayerDescriptor* featureLayerDesc)
{
	for (const auto& featureDesc : featureLayerDesc->featuresDescriptors) {
		// 如果被选中，就先不绘制
		if (featureDesc->isSelected)
			continue;

		switch (featureDesc->type) {
		default:
			break;
		case kPoint:
			drawPoints(featureDesc, pointShader);
			break;
		case kMultiPoint:
			drawMultiPoints(featureDesc, pointShader);
			break;
		case kPolygon:
			drawPolygon(featureDesc, polygonShader, borderShader);
			break;
		case kLineString:
			drawLineString(featureDesc, lineShader);
			break;
		case kMultiPolygon:
			drawMultiPolygon(featureDesc, polygonShader, borderShader);
			break;
		case kMultiLineString:
			drawMultiLineString(featureDesc, lineShader);
			break;
		}
	}
}

/* 绘制栅格图层 */
void OpenglLayerManager::drawRasterLayers(OpenglRasterLayerDescriptor* rasterLayerDesc)
{
	auto rasterDesc = rasterLayerDesc->rasterDesc;
	renderer->DrawTexture(rasterDesc->vao, rasterDesc->ibo, rasterDesc->texs, textureShader);
}


/* 绘制被选中的(要素)图层 */
void OpenglLayerManager::drawSelectedFeatures()
{
	for (const auto& featureDesc : selectedFeatures) {
		switch (featureDesc->type) {
		default:
			break;
		case kPoint:
			drawPoints(featureDesc, highlightShader);
			break;
		case kMultiPoint:
			drawMultiPoints(featureDesc, highlightShader);
			break;
		case kPolygon:
			drawPolygon(featureDesc, polygonShader, highlightShader);
			break;
		case kLineString:
			drawMultiPolygon(featureDesc, polygonShader, highlightShader);
			break;
		case kMultiPolygon:
			drawMultiPolygon(featureDesc, polygonShader, highlightShader);
			break;
		case kMultiLineString:
			drawMultiLineString(featureDesc, highlightShader);
			break;
		}
	}
}

/***************************************************/
/*                                                 */
/*             Private Functions                   */
/*                                                 */
/***************************************************/

void OpenglLayerManager::drawPoints(OpenglFeatureDescriptor* featureDesc, Shader* shader)
{
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawPoint(featureDesc->vao, ibo, shader);
	}
}

void OpenglLayerManager::drawMultiPoints(OpenglFeatureDescriptor* featureDesc, Shader* shader)
{
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawPoint(featureDesc->vao, ibo, shader);
	}
}

void OpenglLayerManager::drawPolygon(OpenglFeatureDescriptor* featureDesc, Shader* fillShader, Shader* borderShader)
{
	// 填充颜色
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawPolygon(featureDesc->vao, ibo, fillShader);
	}
	// 绘制边框
	renderer->DrawPolygonBorder(featureDesc->vao, borderShader);
}

void OpenglLayerManager::drawMultiPolygon(OpenglFeatureDescriptor* featureDesc, Shader* fillShader, Shader* borderShader)
{
	// 填充颜色
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawPolygon(featureDesc->vao, ibo, fillShader);
	}
	// 绘制边框
	renderer->DrawPolygonBorder(featureDesc->vao, borderShader);
}

void OpenglLayerManager::drawLineString(OpenglFeatureDescriptor* featureDesc, Shader* shader)
{
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawLine(featureDesc->vao, ibo, shader);
	}
}

void OpenglLayerManager::drawMultiLineString(OpenglFeatureDescriptor* featureDesc, Shader* shader)
{
	for (const auto& ibo : featureDesc->ibos) {
		renderer->DrawLine(featureDesc->vao, ibo, shader);
	}
}

