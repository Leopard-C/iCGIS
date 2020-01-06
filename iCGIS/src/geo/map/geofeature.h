/*******************************************************************
** class name:  GeoFeature
**
** description: 要素类，一个要素由一个Geometry和若干字段值组成
**
** last change: 2020-01-02
********************************************************************/
#pragma once

#include "geo/geometry/geogeometry.h"
#include "geo/map/geofielddefn.h"
#include <vector>

class GeoFeatureLayer;


class GeoFeature {
public:
	// 构造GeoFeature对象时要指定所属的图层
	// 或者指定表头定义（即GeoField指针数组）
	GeoFeature(GeoFeatureLayer* layerParent);
	GeoFeature(int nFID, GeoFeatureLayer* layerParent);
	GeoFeature(std::vector<GeoFieldDefn*>* fieldDefnsIn);
	GeoFeature(int nFID, std::vector<GeoFieldDefn*>* fieldDefnsIn);
	~GeoFeature();

	/********************************************
	**
	**	geometry
	**
	*********************************************/
	void setGeometry(GeoGeometry* geomIn);
	GeoGeometry* getGeometry() const { return geom; }
	GeometryType getGeometryType() const;


	/********************************************          
	**
	**	field
	**
	*********************************************/

	// 设置字段值前必须确保该字段已经有个初始值
	void initNewFieldValue();

	// 获取字段值
	template<typename T>
	bool getField(QString name, T* outValue) const {
		return getField(getFieldIndexByName(name), outValue);
	}

	template<typename T>
	bool getField(int idx, T* outValue) const {
		if (checkFieldIndex(idx)) {
			*outValue = *(T*)fieldValues[idx];
			return true;
		}
		else {
			return false;
		}
	}

	// 设置字段值
	template<typename T>
	void setField(int idx, T valueIn) {
		if (checkFieldIndex(idx)) {
			initNewFieldValue();
			*(T*)(fieldValues[idx]) = valueIn;
		}
	}

	template<typename T>
	void setField(QString name, T valueIn) {
		setField(getFieldIndexByName(name), valueIn);
	}

	// 每个feature都有一个唯一FID（在所属的图层中唯一）
	int getFID() const { return nFID; }
	void setFID(int nFIDIn) { nFID = nFIDIn; }

	int getNumFields() const { return (*fieldDefns).size(); }
	QString getFieldName(int idx) const;
	GeoFieldType getFieldType(int idx) const;
	GeoFieldType getFieldType(const QString& name) const;

	// 边界
	void updateExtent() { extent = geom->getExtent(); }
	GeoExtent getExtent() const { return extent; }

	// 字段是否存在
	bool isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	// 字段是否存在（模糊匹配）
	bool isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	// 获取字段索引，即字段在第几列
	int getFieldIndexByName(const QString& name) const;

private:
	// 检查下标索引的合法性
	bool checkFieldIndex(int idx) const
		{ return idx > -1 && idx < (*fieldDefns).size(); }
	bool checkFieldName(const QString& name) const;

private:
	// FID 不是属性表中的字段， 但是会显示在属性表中
	int nFID = 0;

	/* 将边界缓存起来，加快获取边界的速度
	** 必要时调用updateExtent()更新 */
	GeoExtent extent;

	// geometry(一个feature只有一个）
	GeoGeometry* geom = nullptr;

	/* Field defination 
	** 每个图层只有一份属性表字段的定义
	** 每个图层中的要素保存一份指针 */
	std::vector<GeoFieldDefn*>* fieldDefns; 
	
	/* 相应字段具体的值
	** 读取数据是根据相应字段的字段定义中的字段类型进行指针的转换 */
	std::vector<void*> fieldValues;
};

