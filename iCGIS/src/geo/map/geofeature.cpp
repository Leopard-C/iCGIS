#include "geo/map/geofeature.h"
#include "geo/map/geolayer.h"


GeoFeature::GeoFeature(GeoFeatureLayer* layerParent) :
	fieldDefns(layerParent->getFieldDefns())
{
	initNewFieldValue();
}


GeoFeature::GeoFeature(std::vector<GeoFieldDefn*>* fieldDefnsIn) :
	fieldDefns(fieldDefnsIn)
{
	initNewFieldValue();
}

GeoFeature::GeoFeature(int nFID, GeoFeatureLayer* layerParent) :
	nFID(nFID), fieldDefns(layerParent->getFieldDefns())
{
	initNewFieldValue();
}

GeoFeature::GeoFeature(int nFID, std::vector<GeoFieldDefn*>* fieldDefnsIn) :
	nFID(nFID), fieldDefns(fieldDefnsIn)
{
	initNewFieldValue();
}

GeoFeature::~GeoFeature()
{
	// 析构geometry
	if (geom)
		delete geom;
	// 析构每个字段的值
	int count = fieldDefns->size();
	for (int i = 0; i < count; ++i) {
		GeoFieldType fieldType = (*fieldDefns)[i]->getType();
		switch (fieldType) {
		case kFieldInt:
			delete (int*)fieldValues[i];
			break;
		case kFieldDouble:
			delete (double*)fieldValues[i];
			break;
		case kFieldText:
			delete (QString*)fieldValues[i];
			break;
		case kFieldUnknown:
			break;
		default:
			break;
		}
	}
}


void GeoFeature::setGeometry(GeoGeometry* geomIn)
{
	if (this->geom)
		delete this->geom;
	this->geom = geomIn;
}

GeometryType GeoFeature::getGeometryType() const
{
	if (!geom)
		return kGeometryTypeUnknown;
	else
		return geom->getGeometryType();
}

void GeoFeature::initNewFieldValue()
{
	int numFieldDefns = fieldDefns->size();
	int numFieldValues = fieldValues.size();
	for (int i = numFieldValues; i < numFieldDefns; ++i) {
		switch ((*fieldDefns)[i]->getType()) {
		default: 
			break;
		case kFieldInt:
			fieldValues.push_back(new int(0));
			break;
		case kFieldDouble:
			fieldValues.push_back(new double(0.0));
			break;
		case kFieldText:
			fieldValues.push_back(new QString());
			break;
		}
	}
}

QString GeoFeature::getFieldName(int idx) const
{
	return (*fieldDefns)[idx]->getName();
}

GeoFieldType GeoFeature::getFieldType(int idx) const
{
	return (*fieldDefns)[idx]->getType();
}

GeoFieldType GeoFeature::getFieldType(const QString& name) const
{
	int idx = getFieldIndexByName(name);
	if (idx != -1)
		return getFieldType(idx);
	else
		return kFieldUnknown;
}

bool GeoFeature::checkFieldName(const QString& name) const
{
	return getFieldIndexByName(name) != -1;
}

// 字段是否存在
bool GeoFeature::isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int count = (*fieldDefns).size();
	for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().compare(fieldName, cs) == 0) {
			return true;
		}
	}
	return false;
}

// 字段是否存在（模糊匹配）
bool GeoFeature::isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int count = (*fieldDefns).size();
	for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().contains(fieldName, cs)) {
			return true;
		}
	}
	return false;
}

int GeoFeature::getFieldIndexByName(const QString& name) const
{
	int count = (*fieldDefns).size();
	for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName() == name) {
			return i;
		}
	}
	return -1;
}


