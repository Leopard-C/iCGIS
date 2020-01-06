#include "geomap.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "utility.h"
#include "logger.h"


GeoMap::GeoMap()
{
}

GeoMap::~GeoMap()
{
	for (auto& layer : layers)
		delete layer;
}

GeoLayer* GeoMap::getLayerByOrder(int order) const
{
	int nLID = layerOrders[order];
	return getLayerByLID(nLID);
}

GeoLayer* GeoMap::getLayerByLID(int nLID) const
{
	int idx = getLayerIndexByLID(nLID);
	if (idx != -1)
		return layers[idx];
	else
		return nullptr;
}

GeoLayer* GeoMap::getLayerByName(const QString& name) const
{
	int idx = getLayerIndexByName(name);
	if (idx != -1)
		return layers[idx];
	else
		return nullptr;
}

// 最后一个Layer
GeoLayer* GeoMap::getLastLayer() const
{
	if (layers.empty())
		return nullptr;
	else
		return layers[layers.size() - 1];
}

int GeoMap::getLayerIndexByOrder(int order) const
{
	int nLID = layerOrders[order];
	return getLayerIndexByLID(nLID);
}

int GeoMap::getLayerIndexByLID(int nLID) const
{
	int count = layers.size();
	for (int i = 0; i < count; ++i) {
		if (layers[i]->getLID() == nLID)
			return i;
	}
	return -1;
}

int GeoMap::getLayerIndexByName(const QString& name) const
{
	int count = layers.size();
	for (int i = 0; i < count; ++i) {
		if (layers[i]->getName() == name)
			return i;
	}
	return -1;
}

// 返回该图层的LID
int GeoMap::addLayer(GeoLayer* layerIn)
{
	// 已有元素order+1（靠后）
	// 新的图层置为顶层
	layerOrders.push_front(currentLID);

	layerIn->setLID(currentLID++);
	// 更新地图范围
	if (isEmpty())
		properties.extent = layerIn->getExtent();
	else
		properties.extent.merge(layerIn->getExtent());
	layers.push_back(layerIn);

	return currentLID - 1;	// 返回新添加的图层的LID，不是在容器中的索引
}

bool GeoMap::removeLayerByLID(int nLID)
{
	int idx = getLayerIndexByLID(nLID);
	if (idx != -1) {
		// 从图层顺序表中清除
		for (auto iter = layerOrders.begin(); iter != layerOrders.end(); ++iter) {
			if (*iter == nLID) {
				layerOrders.erase(iter);
				break;
			}
		}
		// 从图层表中清除
		delete layers[idx];
		layers.erase(layers.begin() + idx);
		updateExtent();
		return true;
	}
	else {
		return false;
	}
}

bool GeoMap::removeLayerByName(const QString& name)
{
	int idx = getLayerIndexByName(name);
	if (idx != -1) {
		// 从图层顺序表中清除
		int nLID = layers[idx]->getLID();
		for (auto iter = layerOrders.begin(); iter != layerOrders.end(); ++iter) {
			if (*iter == nLID) {
				iter = layerOrders.erase(iter);
				break;
			}
		}
		// 从图层表中清除
		delete layers[idx];
		layers.erase(layers.begin() + idx);
		updateExtent();
		return true;
	}
	else {
		return false;
	}
}

// 将 nLID的图层 移动到 insertLID图层的前面
void GeoMap::changeLayerOrder(int nLID, int insertLID)
{
	auto iterNLID = std::find(layerOrders.begin(), layerOrders.end(), nLID);
	auto iterInsertLID = std::find(layerOrders.begin(), layerOrders.end(), insertLID);
	// 提升nLID的顺序
	if (iterNLID > iterInsertLID) {
		for (auto iter = iterNLID; iter != iterInsertLID; --iter) {
			*iter = *(iter - 1);
		}
		*iterInsertLID = nLID;
	}
	// 降低nLID的顺序
	else {
		for (auto iter = iterNLID; iter != iterInsertLID - 1; ++iter) {
			*iter = *(iter + 1);
		}
		*(iterInsertLID - 1) = nLID;
	}
}

bool GeoMap::isEditable() const
{
	for (const auto& layer : layers) {
		if (layer->getLayerType() == kFeatureLayer) {
			if (layer->toFeatureLayer()->isEditable()) {
				return true;
			}
		}
	}
	return false;
}

void GeoMap::updateExtent()
{
	if (isEmpty()) {
		properties.extent = GeoExtent();
	}
	else {
		properties.extent = layers[0]->getExtent();
		int count = layers.size();
		for (int i = 1; i < count; ++i) {
			properties.extent.merge(layers[i]->getExtent());
			// 或者
			//properties.extent += layers[i]->getExtent();
		}
	}
}

