#include "geo/utility/geojson.h"

#include "geo/map/geolayer.h"
#include "geo/geometry/geogeometry.h"

#include <fstream>
#include <QDebug>
#include "logger.h"


bool GeoJson::parse(const std::string& filename, GeoFeatureLayer* layer)
{
	if (!layer) {
		LError("Input layer is null");
		return false;
	}

	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		LError("Open GeoJson Error");
		return false;
	}

	Json::Reader reader;
	Json::Value  root;
	bool ret = false;

	do {
		if (!reader.parse(ifs, root, false))
			break;
		if (root["type"].isNull())
			break;

		std::string type = root["type"].asString();

		if (type == "FeatureCollection") {
			ret = parseGeoJsonTypeFeatureCollection(root, layer);
		}
		else if (type == "GeometryCollection") {
			//ret = parseGeoJsonTypeGeometryCollection(root, layer);
		}
		else if (type == "feature") {
			GeoFeature* feature = new GeoFeature(layer);
			if (ret = parseGeoJsonTypeFeature(root, layer, feature))
				layer->addFeature(feature);
			else
				delete feature;
		}
		else {
			break;
		}

		ret = true;
	} while (false);

	if (!ret) {
		LError("Parse GeoJson Error");
	}

	ifs.close();
	return ret;
}


bool GeoJson::parseGeoJsonTypePoint(const Json::Value& root, GeoPoint* pointOut)
{
	if (root["coordinates"].isNull() || !pointOut)
		return false;
	else {
		Json::Value coordinates = root["coordinates"];
		pointOut->setXY(coordinates[0].asDouble(), coordinates[1].asDouble());
		return true;
	}
}
bool GeoJson::parseGeoJsonTypeMultiPoint(const Json::Value& root, GeoMultiPoint* multiPointOut)
{
	return true;
}

bool GeoJson::parseGeoJsonTypeLineString(const Json::Value& coordinates, GeoLineString* lineStringOut)
{
	if (coordinates.isNull() || !coordinates.isArray() || !lineStringOut)
		return false;

	for (const Json::Value& coordinate : coordinates) {
		lineStringOut->addPoint(coordinate[0].asDouble(), coordinate[1].asDouble());
	}
	return true;
}


bool GeoJson::parseGeoJsonTypeMultiLineString(const Json::Value& root, GeoMultiLineString* multiLineStringOut)
{
	if (root["coordinates"].isNull() || !root["coordinates"].isArray() || !multiLineStringOut)
		return false;
	else {
		Json::Value coordinates = root["coordinates"];
		for (const Json::Value& lineString : coordinates) {
			GeoLineString* lineString = nullptr;
			parseGeoJsonTypeLineString(lineString, lineString);
			multiLineStringOut->addGeometry(lineString);
		}
		return true;
	}
	return true;
}


bool GeoJson::parseGeoJsonTypePolygon(const Json::Value& coordinates, GeoPolygon* polygonOut)
{
	if (coordinates.isNull() || !coordinates.isArray() || !polygonOut)
		return false;

	int numRings = coordinates.size();
	if (numRings == 1) {	// 无孔的
		GeoLinearRing* exteriorRing = new GeoLinearRing();
		for (const Json::Value& coordinate : coordinates[0])
			exteriorRing->addPoint(coordinate[0].asDouble(), coordinate[1].asDouble());
		polygonOut->setExteriorRing(exteriorRing);
	}
	else {		// 有孔的
		polygonOut->reserveInteriorRingsCount(numRings - 1);
		GeoLinearRing* exteriorRing = new GeoLinearRing();
		for (const Json::Value& coordinate : coordinates[0])
			exteriorRing->addPoint(coordinate[0].asDouble(), coordinate[1].asDouble());
		polygonOut->setExteriorRing(exteriorRing);
		for (int i = 1; i < numRings; ++i) {
			GeoLinearRing* interiorRing = new GeoLinearRing();
			for (const Json::Value& coordinate : coordinates[i])
				interiorRing->addPoint(coordinate[0].asDouble(), coordinate[1].asDouble());
			polygonOut->addInteriorRing(interiorRing);
		}
	}
	return true;
}


bool GeoJson::parseGeoJsonTypeMultiPolygon(const Json::Value& coordinates, GeoMultiPolygon* multiPolygonOut)
{
	if (coordinates.isNull() || !coordinates.isArray() || !multiPolygonOut)
		return false;

	int polygonSize = coordinates.size();
	multiPolygonOut->reserveNumGeoms(polygonSize);
	for (const Json::Value& polygonI : coordinates) {
		GeoPolygon* geoPolygon = new GeoPolygon();
		parseGeoJsonTypePolygon(polygonI, geoPolygon);
		multiPolygonOut->addGeometry(geoPolygon);
	}

	return true;
}


bool GeoJson::parseGeoJsonTypeGeometryCollection(const Json::Value& root, GeoGeometryCollection* geometryCollectionOut)
{
	if (root["geometries"].isNull() || !root["geometries"].isArray() || !geometryCollectionOut) {
		return false;
	}
	else {
		Json::Value geometries = root["geometries"];
		for (const Json::Value& geometry : geometries) {

		}
		return true;
	}
}


bool GeoJson::parseGeoJsonTypeFeature(const Json::Value& root, GeoFeatureLayer* geoLayerIn, GeoFeature* geoFeatureOut)
{
	if (root["geometry"].isNull() || root["properties"].isNull() || !geoLayerIn)
		return false;
	else {
		Json::Value geometry = root["geometry"];
		Json::Value properties = root["properties"];
		if (geometry.isNull() || properties.isNull() || geometry["type"].isNull())
			return false;
		std::string geometryType = geometry["type"].asString();

		if (!geoFeatureOut)
			return false;

		/* Properties Table */
		int fieldsCount = geoFeatureOut->getNumFields();
		for (int i = 0; i < fieldsCount; ++i) {
			QByteArray bytes = geoFeatureOut->getFieldName(i).toLocal8Bit();
			const char* fieldName = bytes.data();
			if (properties[fieldName].isNull())
				continue;
			switch (geoFeatureOut->getFieldType(i)) {
			default:
				break;
			case kFieldInt:
				geoFeatureOut->setField(fieldName, properties[fieldName].asInt());
				break;
			case kFieldDouble:
				geoFeatureOut->setField(fieldName, properties[fieldName].asDouble());
				break;
			case kFieldText:
				geoFeatureOut->setField(fieldName, QString::fromStdString(properties[fieldName].asString()));
				break;
			}
		}

		/* Geometry */
		bool ret = false;
		if (geometryType == "Point") {
			GeoPoint* geoPoint = new GeoPoint();
			ret = parseGeoJsonTypePoint(geometry["coordinates"], geoPoint);
			geoFeatureOut->setGeometry(geoPoint);
		}
		else if (geometryType == "LineString") {
			GeoLineString* geoLineString = new GeoLineString();
			ret = parseGeoJsonTypeLineString(geometry["coordinates"], geoLineString);
			geoFeatureOut->setGeometry(geoLineString);
		}
		else if (geometryType == "Polygon") {
			GeoPolygon* geoPolygon = new GeoPolygon();
			ret = parseGeoJsonTypePolygon(geometry["coordinates"], geoPolygon);
			geoFeatureOut->setGeometry(geoPolygon);
		}
		else if (geometryType == "MultiPoint") {
			GeoMultiPoint* geoMultiPoint = new GeoMultiPoint();
			ret = parseGeoJsonTypeMultiPoint(geometry["coordinates"], geoMultiPoint);
			geoFeatureOut->setGeometry(geoMultiPoint);
		}
		else if (geometryType == "MultiLineString") {
			GeoMultiLineString* geoMultiLineString = new GeoMultiLineString();
			ret = parseGeoJsonTypeMultiLineString(geometry["coordinates"], geoMultiLineString);
			geoFeatureOut->setGeometry(geoMultiLineString);
		}
		else if (geometryType == "MultiPolygon") {
			GeoMultiPolygon* geoMultiPolygon = new GeoMultiPolygon();
			ret = parseGeoJsonTypeMultiPolygon(geometry["coordinates"], geoMultiPolygon);
			geoFeatureOut->setGeometry(geoMultiPolygon);
		}
		else
			ret = false;

		return ret;
	}
}

bool GeoJson::parseGeoJsonTypeFeatureCollection(const Json::Value& root, GeoFeatureLayer* layerOut, Json::Value* property /*= nullptr*/)
{
	if (root["features"].isNull() || !root["features"].isArray() || !layerOut)
		return false;

	Json::Value features = root["features"];
	if (features.isNull() || !features.isArray())
		return false;
	int featureCount = features.size();
	if (featureCount == 0)
		return true;

	/* Properties Table */
	Json::Value propertyFields = features[0]["properties"];
	if (propertyFields.isNull())
		return false;

	int fieldCount = propertyFields.size();
	Json::Value::Members mem = propertyFields.getMemberNames();
	for (auto it = mem.begin(); it != mem.end(); ++it) {
		const std::string& keyName = *it;
		switch (propertyFields[keyName].type()) {
		default:
			break;
		case Json::objectValue:
			break;
		case Json::arrayValue:
			break;
		case Json::intValue:
		case Json::uintValue: {
			GeoFieldDefn* fieldDefn = new GeoFieldDefn(keyName.c_str(), 0, kFieldInt);
			layerOut->addField(fieldDefn);
			break;
		}
		case Json::realValue: {
			GeoFieldDefn* fieldDefn = new GeoFieldDefn(keyName.c_str(), 0, kFieldDouble);
			layerOut->addField(fieldDefn);
			break;
		}
		case Json::stringValue: {
			GeoFieldDefn* fieldDefn = new GeoFieldDefn(keyName.c_str(), 16, kFieldText);
			layerOut->addField(fieldDefn);
			break;
		}
		}
	}

	for (const Json::Value& feature : features) {
		GeoFeature* geoFeature = new GeoFeature(layerOut);
		if (!parseGeoJsonTypeFeature(feature, layerOut, geoFeature)) {
			delete geoFeature;
			return false;
		}
		geoFeature->updateExtent();
		layerOut->addFeature(geoFeature);
	}

	return true;
}

