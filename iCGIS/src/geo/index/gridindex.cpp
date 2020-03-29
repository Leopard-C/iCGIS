#include "geo/index/gridindex.h"
#include "geo/utility/geo_math.h"

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

    // clear grids
    std::vector<Grid*>().swap(grids);
}


/* Query grid */
/* Get the grid which contains the point */
void GridIndex::queryGrids(double x, double y, Grid*& gridResult)
{
    for (auto& grid : grids) {
        if (grid->getExtent().contain(x, y)) {
            gridResult = grid;
            return;
        }
    }
}

// Get the grids that intersec the rectangle(extent)
void GridIndex::queryGrids(const GeoExtent& extent, std::vector<Grid*>& gridsResult)
{
    for (auto& grid : grids) {
        if (grid->getExtent().isIntersect(extent)) {
            gridsResult.push_back(grid);
        }
    }
}


// Query
// Point query
// construct a square
// x, y:        square's central point
// halfEdge:    a half of rectangle's length of side
void GridIndex::queryFeature(double x, double y, double halfEdge, GeoFeature*& featureOut)
{
    Grid* inGrid = nullptr;
    queryGrids(x, y, inGrid);
    if (!inGrid) {
        return;
    }

    double left = x - halfEdge;
    double right = x + halfEdge;
    double top = y + halfEdge;
    double bottom = y - halfEdge;
    GeoExtent rect(left, right, bottom, top);

    int count = inGrid->getFeatureCount();
    for (int i = 0; i < count; ++i) {
        GeoFeature* feature = inGrid->getFeature(i);
        if (feature->isDeleted())
            continue;

        switch (feature->getGeometryType()) {
        default:
            break;
        case kPoint:
        {
            GeoPoint* point = feature->getGeometry()->toPoint();
            if (gm::isPointInRect(point->getXY(), rect)) {
                featureOut = feature;
                return;
            }
            break;
        }
        case kLineString:
        {
            GeoLineString* lineString = feature->getGeometry()->toLineString();
            if (gm::isLineStringRectIntersect(lineString, rect)) {
                featureOut = feature;
                return;
            }
            break;
        }
        case kPolygon:
        {
            GeoPolygon* polygon = feature->getGeometry()->toPolygon();
            if (gm::isPointInPolygon({ x, y }, polygon)) {
                featureOut = feature;
                return;
            }
            break;
        }
        case kMultiPoint:
        {
            GeoMultiPoint* multiPoint = feature->getGeometry()->toMultiPoint();
            for (auto iter = multiPoint->begin(); iter != multiPoint->end(); ++iter) {
                if (gm::isPointInRect((*iter)->toPoint()->getXY(), rect)) {
                    featureOut = feature;
                    return;
                }
            }
            break;
        }
        case kMultiLineString:
        {
            GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
            for (auto iter = multiLineString->begin(); iter != multiLineString->end(); ++iter) {
                if (gm::isLineStringRectIntersect((*iter)->toLineString(), rect)) {
                    featureOut = feature;
                    return;
                }
            }
            break;
        }
        case kMultiPolygon:
        {
            GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
            for (auto iter = multiPolygon->begin(); iter != multiPolygon->end(); ++iter) {
                if (gm::isPointInPolygon({ x, y }, (*iter)->toPolygon())) {
                    featureOut = feature;
                    return;
                }
            }
            break;
        }
        }
    }
}

// Box query
void GridIndex::queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresOut)
{
    std::vector<Grid*> grids;
    queryGrids(extent, grids);

    int gridsCount = grids.size();
    for (int i = 0; i < gridsCount; ++i) {
        int featuresCount = grids[i]->getFeatureCount();
        for (int j = 0; j < featuresCount; ++j) {
            GeoFeature* feature = grids[i]->getFeature(j);
            if (feature->isDeleted())
                continue;
            switch (feature->getGeometryType()) {
            default:
                break;
            case kPoint:
            {
                GeoPoint* point = feature->getGeometry()->toPoint();
                if (gm::isPointInRect(point->getXY(), extent)) {
                    if (std::find(featuresOut.begin(), featuresOut.end(), feature) == featuresOut.end()) {
                        featuresOut.push_back(feature);
                    }
                }
                break;
            }
            case kPolygon:
            {
                GeoPolygon* polygon = feature->getGeometry()->toPolygon();
                if (gm::isPolygonRectIntersect(polygon, extent)) {
                    if (std::find(featuresOut.begin(), featuresOut.end(), feature) == featuresOut.end()) {
                        featuresOut.push_back(feature);
                    }
                }
                break;
            }
            case kLineString:
            {
                GeoLineString* lineString = feature->getGeometry()->toLineString();
                if (gm::isLineStringRectIntersect(lineString, extent)) {
                    if (std::find(featuresOut.begin(), featuresOut.end(), feature) == featuresOut.end()) {
                        featuresOut.push_back(feature);
                    }
                }
                break;
            }
            case kMultiPoint:
            {
                GeoMultiPoint* multiPoint = feature->getGeometry()->toMultiPoint();
                for (auto iter = multiPoint->begin(); iter != multiPoint->end(); ++iter) {
                    if (gm::isPointInRect((*iter)->toPoint()->getXY(), extent)) {
                        if (std::find(featuresOut.begin(), featuresOut.end(), feature) == featuresOut.end()) {
                            featuresOut.push_back(feature);
                        }
                    }
                }
                break;
            }
            case kMultiLineString:
            {
                GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
                for (auto iter = multiLineString->begin(); iter != multiLineString->end(); ++iter) {
                    if (gm::isLineStringRectIntersect((*iter)->toLineString(), extent)) {
                        featuresOut.push_back(feature);
                        return;
                    }
                }
                break;
            }
            case kMultiPolygon:
            {
                GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
                for (auto iter = multiPolygon->begin(); iter != multiPolygon->end(); ++iter) {
                    if (gm::isPolygonRectIntersect((*iter)->toPolygon(), extent)) {
                        if (std::find(featuresOut.begin(), featuresOut.end(), feature) == featuresOut.end()) {
                            featuresOut.push_back(feature);
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
