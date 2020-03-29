/*******************************************************
** class name:  GridIndex
**
** description: Saticl Grid index
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/index/grid.h"
#include "geo/index/spatialindex.h"


class GridIndex : public SpatialIndex {
public:
    GridIndex();
    ~GridIndex();

    // Reserve the number of grids
    void reserve(int numGrids) { grids.reserve(numGrids); }

    // Add grids
    void addGrid(Grid* grid) { grids.push_back(grid); }

    // Clear all grids
    void clear();

    // Get the number of grids
    int getNumGrids() const { return grids.size(); }

    // Query grid
    // Get the grid which contains the point
    void queryGrids(double x, double y, Grid*& gridResult);

    // Get the grids that intersec the rectangle(extent)
    void queryGrids(const GeoExtent& extent, std::vector<Grid*>& gridsResult);

    // Query
    // Point query
    // construct a square
    // x, y:        square's central point
    // halfEdge:    a half of rectangle's length of side
    void queryFeature(double x, double y, double halfEdge, GeoFeature*& featureResult) override;

    // Box query
    void queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) override;

private:
    std::vector<Grid*> grids;
};
