/*******************************************************
** description: Topolygon analysis
**
** last change: 2020-03-25
*******************************************************/
#pragma once

#include "geo/geometry/geogeometry.h"
#include "geo/geo_base.hpp"

namespace gm {

using Rect = GeoExtent;

/* Compare float numbers */
int dcmp(double x, double y, double precision = 1e-6);

/* Cross product */
double cross(const GeoRawPoint& vec1Start, const GeoRawPoint& vec1End, const GeoRawPoint& vec2Start, const GeoRawPoint& vec2End);

/* Rotate */
void rotate(double centerX, double centerY, double& x, double& y, double sinAngle, double cosAngle);
void rotate(double centerX, double centerY, double& x, double& y, double angle);

/* Point Distance */
double distancePointToPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB);

/* Point & Point */
bool isPointEqPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB, double precision = 2);

/* Point & Line */
bool isPointOnLine(const GeoRawPoint& pt, const GeoRawPoint& lineStartPt, const GeoRawPoint& lineEndPt, double precision = 0.001);
bool isPointOnLineString(const GeoRawPoint& pt, double halfEdge, const GeoRawPoint& lineStartPt, const GeoRawPoint& lineEndPt);
bool isPointOnLineString(const GeoRawPoint& pt, GeoLineString* lineString, double precision = 0.001);

/* Point & Linearing */
bool isPointInLinearRing(const GeoRawPoint& pt, GeoLinearRing* ring, double precision = 0.001);

/* Point & Rectangle */
bool isPointInRect(const GeoRawPoint& point, const Rect& rect);

/* Point & Polygon */
bool isPointInPolygon(const GeoRawPoint& pt, GeoPolygon* polygon, double precision = 0.001);

/* Line & Line */
bool isLineIntersect(const GeoRawPoint& line1Start, const GeoRawPoint& line1End, 
	const GeoRawPoint& line2Start, const GeoRawPoint& line2End, double precision = 1e-6);

/* Rectangle & Rectangle */
bool isRectIntersect(const Rect& rect1, const Rect& rect2);

/* LineString & Rectangle */
bool isLineStringRectIntersect(GeoLineString* lineString, const Rect& rect);

/* Polygon & Polygon */
bool isPolygonRectIntersect(GeoPolygon* polygon, const Rect& rect);


} // namespace gm
