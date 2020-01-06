/*******************************************************
** description: 一系列拓扑分析函数
**				计算点、线、面之间的拓扑关系
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/geometry/geogeometry.h"
#include "geo/geo_base.hpp"

using Rect = GeoExtent;


/* 浮点数比较 */
int dcmp(double x, double y, double precision = 1e-6);

/* 叉积 */
double cross(const GeoRawPoint& vec1Start, const GeoRawPoint& vec1End, const GeoRawPoint& vec2Start, const GeoRawPoint& vec2End);

/* 两点间的距离 */
double distancePointToPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB);

/* 两点是否相等（给定精度下） */
bool isPointEqPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB, double precision = 2);

/* 点是否在线上 */
bool isPointOnLine(const GeoRawPoint& pt, const GeoRawPoint& lineStartPt, const GeoRawPoint& lineEndPt, double precision = 0.001);
bool isPointOnLineString(const GeoRawPoint& pt, GeoLineString* lineString, double precision = 0.001);

/* Point & LinearRing */
bool isPointInLinearRing(const GeoRawPoint& pt, GeoLinearRing* ring, double precision = 0.001);

/* Point & Polygon */
bool isPointInPolygon(const GeoRawPoint& pt, GeoPolygon* polygon, double precision = 0.001);

/* 两线是否相交 */
bool isLineIntersect(const GeoRawPoint& line1Start, const GeoRawPoint& line1End, 
	const GeoRawPoint& line2Start, const GeoRawPoint& line2End, double precision = 1e-6);

/* 两个矩形是否相交 */
bool isRectIntersect(const Rect& rect1, const Rect& rect2);

/* 多段线与矩形相交 */
bool isLineStringRectIntersect(GeoLineString* lineString, const Rect& rect);

/* 多边形与矩形相交 */
bool isPolygonRectIntersect(GeoPolygon* polygon, const Rect& rect);

