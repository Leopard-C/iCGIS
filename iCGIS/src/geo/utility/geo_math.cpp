#include "geo/utility/geo_math.h"
#include <cmath>

namespace gm {

/* Compare float numbers */
int dcmp(double x, double y, double precision /*= 1e-6*/)
{
    if (fabs(x - y) < precision)
        return 0;
    else if (x < y)
        return -1;
    else
        return 1;
}

/* Cross Product */
double cross(const GeoRawPoint& vec1Start, const GeoRawPoint& vec1End, const GeoRawPoint& vec2Start, const GeoRawPoint& vec2End)
{
    return (vec1End.x - vec1Start.x) * (vec2End.y - vec2Start.y) -
           (vec1End.y - vec1Start.y) * (vec2End.x - vec2Start.x);
}

/* Rotate */
void rotate(double centerX, double centerY, double& x, double& y, double sinAngle, double cosAngle) {
    double xOffset = x - centerX;
    double yOffset = y - centerY;
    x = xOffset * cosAngle + yOffset * sinAngle + centerX;
    y = yOffset * cosAngle - xOffset * sinAngle + centerY;
}

void rotate(double centerX, double centerY, double& x, double& y, double angle) {
    rotate(centerX, centerY, x, y, sin(angle * PI / 180.0), cos(angle * PI / 180.0));
}

/* Point Distance */
double distancePointToPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB) {
    return sqrt((ptA.x - ptB.x) * (ptA.x - ptB.x) + (ptA.y - ptB.y) * (ptA.y - ptB.y));
}

// Point & Point
bool isPointEqPoint(const GeoRawPoint& ptA, const GeoRawPoint& ptB, double precision /*= 2*/)
{
    double dis = distancePointToPoint(ptA, ptB);
    if (dis <= precision)
        return true;
    else
        return false;
}

// Point & Line
bool isPointOnLine(const GeoRawPoint& pt, const GeoRawPoint& lineStartPt, const GeoRawPoint& lineEndPt, double precision /*= 0.001*/)
{
    if (fabs(cross(pt, lineStartPt, pt, lineEndPt)) < precision) {
        if ((lineStartPt.x < pt.x && lineEndPt.x > pt.x) || (lineEndPt.x < pt.x && lineStartPt.x > pt.x)) {
            if ((lineStartPt.y < pt.y && lineEndPt.y > pt.y) || (lineEndPt.y < pt.y && lineStartPt.y > pt.y)) {
                return true;
            }
        }
    }
    return false;
}

// Point & Line
//  @pt: the point is the central point of a rectangle
//  @halfEdge: the half of the edge of the rectangle
bool isPointOnLineString(const GeoRawPoint& pt, double halfEdge, GeoLineString* lineString) {
    Rect rect(pt.x - halfEdge, pt.x + halfEdge, pt.y - halfEdge, pt.y + halfEdge);
    return isLineStringRectIntersect(lineString, rect);
}

// Point & LineString
bool isPointOnLineString(const GeoRawPoint& pt, GeoLineString* line, double precision /*= 0.001*/)
{
    int pointCount = line->getNumPoints();
    for (int i = 0; i < pointCount - 1; ++i) {
        if (isPointOnLine(pt, (*line)[i], (*line)[i + 1])) {
            return true;
        }
    }
    return false;
}

// Point & LinearRing
bool isPointInLinearRing(const GeoRawPoint& pt, GeoLinearRing* pRing, double precision)
{
    const GeoLinearRing& ring = *pRing;
    int pointsCount = ring.getNumPoints();
    double maxX = ring[0].x;
    for (int i = 1; i < pointsCount; ++i) {
        if (ring[i].x > maxX)
            maxX = ring[i].x;
    }
    const GeoRawPoint& ptLeft = pt;
    GeoRawPoint ptRight(maxX + 10, pt.y);   // 10 is just a postive number

    // number of intersections
    bool flag = false;
    for (int i = 0, j = pointsCount - 1; i < pointsCount; j = i++) {
        if (isPointOnLine(pt, ring[i], ring[j]))
            return true;
        if (dcmp(ring[i].y, ring[j].y) == 0)
            continue;
        if (isLineIntersect(ptLeft, ptRight, ring[i], ring[j])) {
            if (isPointOnLine(ring[i], ptLeft, ptRight)) {
                if (ring[j].y > pt.y) {
                    flag = !flag;
                }
            }
            else if (isPointOnLine(ring[j], ptLeft, ptRight)) {
                if (ring[i].y > pt.y) {
                    flag = !flag;
                }
            }
            else {
                flag = !flag;
            }
        }
    }
    return flag;
}

/* Point & Rectangle */
bool isPointInRect(const GeoRawPoint& point, const Rect& rect) {
    return point.x > rect.minX && point.x < rect.maxX
           && point.y > rect.minY && point.y < rect.maxY;
}

/* Point & Polygon */
bool isPointInPolygon(const GeoRawPoint& pt, GeoPolygon* pPolygon, double precision /*= 0.001*/)
{
    const GeoPolygon& polygon = *pPolygon;
    /* Judge if the point is in the interior ring */
    int interiorRingsCount = polygon.getInteriorRingsCount();
    GeoLinearRing* interiorRing;
    for (int i = 0; i < interiorRingsCount; ++i) {
        interiorRing = polygon.getInteriorRing(i);
        if (isPointInLinearRing(pt, interiorRing)) {
            return false;
        }
    }

    /* Judge if the point is int the exterior ring */
    GeoLinearRing* exteriorRing = polygon.getExteriorRing();
    if (exteriorRing) {
        if (isPointInLinearRing(pt, exteriorRing)) {
            return true;
        }
    }

    return false;
}


// Line & Line
bool isLineIntersect(const GeoRawPoint& line1Start, const GeoRawPoint& line1End, const GeoRawPoint& line2Start, const GeoRawPoint& line2End, double precision)
{
    double result1 = cross(line2Start, line1Start, line2Start, line2End) * cross(line2Start, line1End, line2Start, line2End);
    double result2 = cross(line1Start, line2Start, line1Start, line1End) * cross(line1Start, line2End, line1Start, line1End);
    if (result1 <= 0 && result2 <= 0)
        return true;
    else
        return false;
}


// Rectangle & Rectangle
bool isRectIntersect(const Rect& rect1, const Rect& rect2)
{
    return rect1.isIntersect(rect2);
}


// LineString & Rectangle
bool isLineStringRectIntersect(GeoLineString* pLineString, const Rect& rect)
{
    const GeoLineString& lineString = *pLineString;
    int pointsCount = lineString.getNumPoints();

    // If any line intersects the rectangle, return true
    for (int i = 0; i < pointsCount; ++i) {
        if (rect.contain(lineString[i]))
            return true;
    }

    GeoRawPoint ptLeftBottom(rect.minX, rect.minY);
    GeoRawPoint ptRightBottom(rect.maxX, rect.minY);
    GeoRawPoint ptRightTop(rect.maxX, rect.maxY);
    GeoRawPoint ptLeftTop(rect.minX, rect.maxY);
    for (int i = 0; i < pointsCount - 1; ++i) {
        if (isLineIntersect(ptLeftBottom, ptRightBottom, lineString[i], lineString[i + 1])
            || isLineIntersect(ptRightBottom, ptRightTop, lineString[i], lineString[i + 1])
            || isLineIntersect(ptRightTop, ptLeftTop, lineString[i], lineString[i + 1])
            || isLineIntersect(ptLeftTop, ptLeftBottom, lineString[i], lineString[i + 1]))
        {
            return true;
        }
    }

    return false;
}


// Polygon & Rectangle
bool isPolygonRectIntersect(GeoPolygon* pPolygon, const Rect& rect)
{
    const GeoPolygon& polygon = *pPolygon;
    // Exterior Ring
    GeoLinearRing* exteriorRing = polygon.getExteriorRing();
    if (exteriorRing) {
        if (isLineStringRectIntersect(exteriorRing->toLineString(), rect)) {
            return true;
        }
    }

    // Interior Ring
    int interiorRingsCount = polygon.getInteriorRingsCount();
    GeoLinearRing* interiorRing;
    for (int i = 0; i < interiorRingsCount; ++i) {
        interiorRing = polygon.getInteriorRing(i);
        if (interiorRing) {
            if (isLineStringRectIntersect(interiorRing->toLineString(), rect)) {
                return true;
            }
        }
    }

    return false;
}

} // namespace gm
