/*******************************************************
** class name:  GeoFeatureLayerProperty
**
** description: 图层属性类
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/map/geofielddefn.h"
#include "geo/geometry/geogeometry.h"

#include <QString>


enum LayerStyleMode {
	kSingleStyle = 0,
	kCategorized = 1,
};


class GeoFeatureLayerProperty {
public:
	GeoFeatureLayerProperty() = default;
	~GeoFeatureLayerProperty() = default;

	void setName(const QString& nameIn) { name = nameIn; }

	GeometryType getGeometryType() const { return geometryType; }
	void setGeometryType(GeometryType geometryTypeIn);

public:
	bool visable = true;		// 是否可见（显示）
	bool editable = false;		// 是否可编辑
	int id = 0;
	QString name;
	GeoExtent extent;
	QString spatialRef;
	LayerStyleMode styleMode = kSingleStyle;
private:
	GeometryType geometryType = kGeometryTypeUnknown;
};
