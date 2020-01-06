/*******************************************************
** class name:  Grid
**
** description: 格网索引中每个格子的类
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/geo_base.hpp"
#include "geo/map/geofeature.h"

#include <vector>

class Grid {
public:
	Grid(int id) : id(id) {}
	Grid(int id, const GeoExtent& extent)
		: id(id), extent(extent) {}

	int getId() const { return id; }
	const GeoExtent& getExtent() const { return extent; }
	GeoFeature* getFeature(int idx) const { return featuresList[idx]; }
	int getFeatureCount() const { return featuresList.size(); }

	void setExtent(const GeoExtent& extentIn) { extent = extentIn; }
	void addFeature(GeoFeature* feature) { featuresList.push_back(feature); }

private:
	int id;
	GeoExtent extent;
	std::vector<GeoFeature*> featuresList;
};
