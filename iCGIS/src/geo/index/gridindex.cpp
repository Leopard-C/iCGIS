#include "geo/index/gridindex.h"
#include "geo/utility/geo_math.h"

#include <iostream>
#include <algorithm>


GridIndex::GridIndex()
{

}

GridIndex::~GridIndex()
{
	clear();
}


void GridIndex::clear()
{
	if (grids.empty())
		return;

	for (auto& grid : grids)
		delete grid;
	
	grids.clear();
}

/* 点查询 ： 格网 */
void GridIndex::queryGrids(double x, double y, Grid*& gridResult)
{
	for (auto& grid : grids) {
		if (grid->getExtent().contain(x, y)) {
			gridResult = grid;
			return;
		}
	}
}

/* 矩形框查询 ：格网 */
void GridIndex::queryGrids(const GeoExtent& extent, std::vector<Grid*>& gridsResult)
{
	for (auto& grid : grids) {
		if (grid->getExtent().isIntersect(extent)) {
			gridsResult.push_back(grid);
		}
	}
}

/* 点查询 ：要素 */
void GridIndex::queryFeatures(double x, double y, GeoFeature*& featureResult)
{
	Grid* inGrid = nullptr;
	queryGrids(x, y, inGrid);
	if (!inGrid) {
		return;
	}

	std::cout << "in Grid: " << inGrid->getId() << std::endl;

	int count = inGrid->getFeatureCount();
	for (int i = 0; i < count; ++i) {
		GeoFeature* feature = inGrid->getFeature(i);
		switch (feature->getGeometryType()) {
		default:
			break;
		case kPoint:
		{
			break;
		}
		case kPolygon:
		{
			GeoPolygon* polygon = feature->getGeometry()->toPolygon();
			if (isPointInPolygon({ x, y }, polygon)) {
				featureResult = feature;
				return;
			}
			break;
		}
		case kLineString:
		{
			break;
		}
		case kMultiPoint:
		{
			break;
		}
		case kMultiLineString:
		{
			break;
		}
		case kMultiPolygon:
		{
			GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
			int polygonsCount = multiPolygon->getNumGeometries();
			GeoPolygon* polygon = nullptr;
			for (int j = 0; j < polygonsCount; ++j) {
				polygon = multiPolygon->getGeometry(j)->toPolygon();
				if (isPointInPolygon({ x, y }, polygon)) {
					featureResult = feature;
					return;
				}
			}
			break;
		}
		}
	}
}

/* 矩形查询 ：要素 */
void GridIndex::queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult)
{
	std::vector<Grid*> grids;
	queryGrids(extent, grids);

	int gridsCount = grids.size();
	for (int i = 0; i < gridsCount; ++i) {
		int featuresCount = grids[i]->getFeatureCount();
		for (int j = 0; j < featuresCount; ++j) {
			GeoFeature* feature = grids[i]->getFeature(j);
			switch (feature->getGeometryType()) {
			default:
				break;
			case kPoint:
			{
				break;
			}
			case kPolygon:
			{
				GeoPolygon* polygon = feature->getGeometry()->toPolygon();
				if (isPolygonRectIntersect(polygon, extent)) {
					featuresResult.push_back(feature);
				}
				break;
			}
			case kLineString:
			{
				break;
			}
			case kMultiPoint:
			{
				break;
			}
			case kMultiLineString:
			{
				break;
			}
			case kMultiPolygon:
			{
				GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
				int polygonsCount = multiPolygon->getNumGeometries();
				GeoPolygon* polygon = nullptr;
				for (int k = 0; k < polygonsCount; ++k) {
					polygon = multiPolygon->getGeometry(k)->toPolygon();
					if (isPolygonRectIntersect(polygon, extent)) {
						if (std::find(featuresResult.begin(), featuresResult.end(), feature) == featuresResult.end()) {
							featuresResult.push_back(feature);
						}
						break;
					}
				}
				break;
			}
			}	// end switch
		} // end for j
	} // end for i
}

