/*************************************************************
** class name:  GeoJson
**
** description: Parse json file using JsonCPP library
**
** last change: 2020-01-02
*************************************************************/
#pragma once

#include <string>
#include <jsoncpp/json/json.h>

#include "geo/map/geolayer.h"
#include "geo/geometry/geogeometry.h"


class GeoJson {
public:
    bool parse(const std::string& filename, GeoFeatureLayer* layer);

private:
    bool parseGeoJsonTypePoint(const Json::Value& root, GeoPoint* pointOut);
    bool parseGeoJsonTypeMultiPoint(const Json::Value& root, GeoMultiPoint* multiPointOut);
    bool parseGeoJsonTypeLineString(const Json::Value& root, GeoLineString* lineStringOut);
    bool parseGeoJsonTypeMultiLineString(const Json::Value& root, GeoMultiLineString* multiLineStringOut);
    bool parseGeoJsonTypePolygon(const Json::Value& root, GeoPolygon* polygonOut);
    bool parseGeoJsonTypeMultiPolygon(const Json::Value& root, GeoMultiPolygon* multiPolygonOut);
    bool parseGeoJsonTypeGeometryCollection(const Json::Value& root, GeoGeometryCollection* geometryCollectionOut);
    bool parseGeoJsonTypeFeature(const Json::Value& root, GeoFeatureLayer* geoLayerIn, GeoFeature* geoFeatureOut);
    bool parseGeoJsonTypeFeatureCollection(const Json::Value& root, GeoFeatureLayer* layer, Json::Value* property = nullptr);
};
