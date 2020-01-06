/****************************************************************
** base class:  GeoLayer
**
** sub classes: GeoFeatureLayer
**				GeoRasterLayer
**
** description: 图层类（要素图层[矢量]、栅格图层[栅格]）
**
** last change: 2020-01-04
*****************************************************************/
#pragma once

#include "geo/map/geofeature.h"
#include "geo/map/geofeaturelayerproperty.h"
#include "geo/map/georasterlayerproperty.h"
#include "geo/index/gridindex.h"
#include "geo/raster/georasterdata.h"

#include <vector>
#include <QStringList>

class GeoRasterLayer;
class GeoFeatureLayer;

enum LayerType {
	kRasterLayer = 0,
	kFeatureLayer    = 1
};


/**********************************************/
/*                                            */
/*         图层基类                           */
/*                                            */
/**********************************************/

class GeoLayer {
public:
	GeoLayer() {}
	virtual ~GeoLayer() {}

public:
	virtual LayerType getLayerType() const = 0;
	virtual GeoExtent getExtent() const = 0;
	virtual QString getName() const = 0;
	virtual int getLID() const = 0;
	virtual bool isVisable() const = 0;

	virtual void setName(const QString& name) = 0;
	virtual void setLID(int nLID) = 0;
	virtual void setExtent(const GeoExtent& extent) = 0;
	virtual void setVisable(bool visable) = 0;

public:
	inline GeoFeatureLayer* toFeatureLayer() { return utils::down_cast<GeoFeatureLayer*>(this); }
	inline GeoRasterLayer* toRasterLayer() { return utils::down_cast<GeoRasterLayer*>(this); }
};



/***********************************************/
/*                                             */
/*          要素图层（矢量图层）                */
/*                                             */
/***********************************************/

class GeoFeatureLayer : public GeoLayer {
	friend class GeoFeatureLayerProperty;
public:
	GeoFeatureLayer();
	GeoFeatureLayer(int nLID);
	GeoFeatureLayer(int nLID, GeometryType type);
	virtual ~GeoFeatureLayer();

	/******************************
	**  Feature				
	*****************************/
	bool isEmpty() const { return features.empty(); }
	void reserveFeatureCount(int count) { features.reserve(count); }
	size_t getFeatureCount() const { return features.size(); }
	GeoFeature* getFeatureByFID(int nFID) const;
	GeoFeature* getFeature(int idx) const { return features[idx]; }
	GeometryType getGeometryType() const { return properties.getGeometryType(); }

	void setGeometryType(GeometryType typeIn) { properties.setGeometryType(typeIn); }
	bool addFeature(GeoFeature* feature);

	template<typename T>
	GeoFeature* getFeatureByFieldValue(int fieldIndex, T value) {
		if (fieldIndex < fieldDefns->size()) {
			int featureCount = features.size();
			for (int i = 0; i < featureCount; ++i) {
				T valueF;
				features[i]->getField(fieldIndex, &valueF);
				if (valueF == value)
					return features[i];
			}
		}
		return nullptr;
	}
	
	std::vector<GeoFeature*>::iterator begin() { return features.begin(); }
	std::vector<GeoFeature*>::iterator end() { return features.end(); }


	/******************************
	**  FieldDefn				
	*****************************/
	std::vector<GeoFieldDefn*>* getFieldDefns() const
		{ return fieldDefns; }
	GeoFieldDefn* getFieldDefn(int idx) const { return (*fieldDefns)[idx]; }
	GeoFieldDefn* getFieldDefn(const QString& name) const;
	int getFieldIndex(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	int getFieldIndexLike(const QString& name, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	int getNumFields() const { return fieldDefns->size(); }
	void reserveFieldCount(int count) { fieldDefns->reserve(count); }
	QStringList getFieldList() const;

	int addField(const QString& nameIn, int widthIn, GeoFieldType typeIn);
	int addField(GeoFieldDefn* fieldDefnIn);
	bool isFieldExist(GeoFieldDefn* fieldDefn);
	bool GeoFeatureLayer::isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	bool GeoFeatureLayer::isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;


	/******************************
	**  Property
	*****************************/
	int getLID() const override { return properties.id; }
	LayerType getLayerType() const override { return kFeatureLayer; }
	QString getName() const override { return properties.name; }
	GeoExtent getExtent() const override { return properties.extent; }
	bool isVisable() const override { return properties.visable; }
	bool isEditable() const { return properties.editable; }
	QString getSpatialRef() const { return properties.spatialRef; }
	LayerStyleMode getStyleMode() const { return properties.styleMode; }

	void setLID(int nLIDIn) override { properties.id = nLIDIn; }
	void setName(const QString& nameIn) override { properties.setName(nameIn); }
	void setExtent(const GeoExtent& extentIn) override { properties.extent = extentIn; }
	void updateExtent();
	void setVisable(bool visableIn) override { properties.visable = visableIn; }
	void setEditable(bool editableIn) { properties.editable = editableIn; }
	void setSpatialRef(const QString& spatialRefIn) { properties.spatialRef = spatialRefIn; }
	void setStyleMode(LayerStyleMode mode) { properties.styleMode = mode; };

	/******************************
	** Spatial Index
	******************************/
	bool createGridIndex();
	void queryFeatures(double x, double y, GeoFeature*& featureResult) const;
	void queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) const;
	
private:
	/* 最后一个feature的FID的下一个FID */
	/* 每次新增feature时currentFID自增1 */
	int currentFID = 0;		

	std::vector<GeoFeature*> features;
	std::vector<GeoFieldDefn*>* fieldDefns = nullptr;
	GeoFeatureLayerProperty properties;

	/* Index */
	// grid index
	GridIndex* gridIndex = nullptr;
};



/***********************************************/
/*                                             */
/*              栅格图层                       */
/*                                             */
/***********************************************/

class GeoRasterLayer : public GeoLayer {
public:
	GeoRasterLayer() {}
	GeoRasterLayer(int nLID) { properties.id = nLID; }
	virtual ~GeoRasterLayer();

	/************************
	* Data
	************************/
	void setData(GeoRasterData* pDataIn);
	GeoRasterData* getData() const { return pData; }


	/************************
	* Property
	************************/
	virtual GeoExtent getExtent() const override { return properties.extent; }
	virtual LayerType getLayerType() const override { return kRasterLayer; }
	virtual QString getName() const override { return properties.name; }
	virtual int getLID() const { return properties.id; }
	virtual bool isVisable() const { return properties.visable; }

	virtual void setName(const QString& name) { properties.name = name; }
	virtual void setLID(int nLID) { properties.id = nLID; }
	virtual void setExtent(const GeoExtent& extent) { properties.extent = extent; }
	virtual void setVisable(bool visableIn) { properties.visable = visableIn; }

private:
	GeoRasterData* pData = nullptr;
	GeoRasterLayerProperty properties;
};
