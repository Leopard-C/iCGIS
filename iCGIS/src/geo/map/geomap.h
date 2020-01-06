/*******************************************************
** class name:  GeoMap
**
** description: 地图类，地图由若干图层组成
**				每个图层都有个唯一的LID， 只增不减			
**				每次添加新图层，currentLID自增1			
**				图层的LID是固定的，但索引值(下标)可能会变	
**
** last change: 2020-01-04
*******************************************************/
#pragma once

#include "geo/map/geolayer.h"
#include "geo/map/geomapproperty.h"

#include <map>
#include <deque>
#include <vector>


class GeoMap {
public:
	// 图层显示顺序 结构体
	struct LayerOrder {
		LayerOrder(int nLID, int nOrder)
			: LID(nLID), order(nOrder) {}
		int LID;
		int order;
	};

public:
	GeoMap();
	~GeoMap();

	/******************************
	**  GeoLayer				
	*****************************/
	bool isEmpty() const { return layers.empty(); }
	int getNumLayers() const { return layers.size(); }
	GeoLayer* getLayerByOrder(int order) const;
	GeoLayer* getLayerById(int idx) const { return layers[idx]; }
	GeoLayer* getLayerByLID(int nLID) const;
	GeoLayer* getLayerByName(const QString& name) const;
	GeoLayer* getLastLayer() const;
	int getLayerLIDByOrder(int order) const
		{ return layerOrders[order]; }
	int getLayerIndexByOrder(int order) const;
	int getLayerIndexByLID(int lid) const;
	int getLayerIndexByName(const QString& name) const;

	// 添加图层，返回该图层的LID
	int addLayer(GeoLayer* layerIn);
	// 删除图层
	bool removeLayerByLID(int nLID);
	bool removeLayerByName(const QString& name);
	// 将nLID的图层移动到insertLID图层的前面
	void changeLayerOrder(int nLID, int insertLID);

	bool checkLayerIndex(int idx) const
		{ return idx > -1 && idx < layers.size(); }

	std::vector<GeoLayer*>::iterator begin() { return layers.begin(); }
	std::vector<GeoLayer*>::iterator end() { return layers.end(); }


	/******************************
	**  Property	
	*****************************/
	QString getName() const { return properties.name; }
	GeoExtent getExtent() const { return properties.extent; }
	bool isEditable() const;

	void setName(const QString& nameIn) { properties.name = nameIn; }
	void updateExtent();

private:
	/* 当前地图中最后一个图层的LID的下一个LID */
	/* 每次新增图层是自增1 */	
	int currentLID = 0;

	// 图层列表
	std::vector<GeoLayer*> layers;
	
	// 地图属性
	GeoMapProperty properties;

	// layerOrders[n] 表示顺序为n的图层的LID
	// 通过该下标获取到相应图层
	std::deque<int> layerOrders;
};

