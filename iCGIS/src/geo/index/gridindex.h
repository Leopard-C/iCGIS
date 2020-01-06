/*******************************************************
** class name:  GridIndex
**
** description: 空间网格索引
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

	/* 预分配网格数量 */
	void reserve(int numGrids) { grids.reserve(numGrids); }

	/* 添加网格 */
	void addGrid(Grid* grid) { grids.push_back(grid); }

	/* 清空索引 */
	void clear();

	/* 获取格网数量 */
	int getNumGrids() const { return grids.size(); }

	/* 查询格网 */
	void queryGrids(double x, double y, Grid*& gridResult);	// 点查询
	void queryGrids(const GeoExtent& extent, std::vector<Grid*>& gridsResult);	// 矩形框查询

	/* 查询空间要素 */
	void queryFeatures(double x, double y, GeoFeature*& featureResult) override;
	void queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) override;

private:
	std::vector<Grid*> grids;
};
