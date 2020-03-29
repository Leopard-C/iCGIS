#pragma once

#include "util/utility.h"


/**************************************************/
/*                                                */
/*              GeoRawPoint                       */
/*                                                */
/**************************************************/
class GeoRawPoint {
public:
	GeoRawPoint() : x(0.0), y(0.0) {}
	GeoRawPoint(double xx, double yy) : x(xx), y(yy) {}
	bool operator==(const GeoRawPoint& rhs)
		{ return utils::isEqual(x, rhs.x) && utils::isEqual(y, rhs.y); }
	double x;
	double y;
};


/**************************************************/
/*                                                */
/*               GeoExtent                        */
/*		   minX, maxX, minY, maxY                 */
/*                                                */
/**************************************************/
class GeoExtent {
public:
	GeoExtent()
		: minX(0.0), maxX(0.0), minY(0.0), maxY(0.0) {}
	GeoExtent(double xx, double yy)
		: minX(xx), maxX(xx), minY(yy), maxY(yy) {}
	GeoExtent(const GeoRawPoint& rawPt)
		: minX(rawPt.x), maxX(rawPt.x), minY(rawPt.y), maxY(rawPt.y) {}
	GeoExtent(const GeoRawPoint& leftTop, const GeoRawPoint rightBottom)
		: minX(leftTop.x), maxX(rightBottom.x), minY(rightBottom.y), maxY(leftTop.y) {}
	GeoExtent(double minXIn, double maxXIn, double minYIn, double maxYIn) :
		minX(minXIn), maxX(maxXIn), minY(minYIn), maxY(maxYIn) {}

	double centerX() const { return (maxX + minX) / 2; }
	double centerY() const { return (maxY + minY) / 2; }
	double width()	 const { return maxX - minX; }
	double height()  const { return maxY - minY; }
	double aspectRatio() const { return (maxX - minX) / (maxY - minY); }
	GeoRawPoint center() const { return GeoRawPoint((maxX + minX) / 2, (maxY + minY) / 2); }

	GeoExtent operator+(const GeoExtent& rhs) {
		GeoExtent extent;
		extent.minX = MIN(minX, rhs.minX);
		extent.maxX = MAX(maxX, rhs.maxX);
		extent.minY = MIN(minY, rhs.minY);
		extent.maxY = MAX(maxY, rhs.maxY);
		return extent;
	}

	GeoExtent& operator+=(const GeoExtent& rhs) {
		merge(rhs);
		return *this;
	}

	void merge(const GeoExtent& rhs) {
		minX = MIN(minX, rhs.minX);
		minY = MIN(minY, rhs.minY);
		maxX = MAX(maxX, rhs.maxX);
		maxY = MAX(maxY, rhs.maxY);
	}

	void merge(double xx, double yy) {
		minX = MIN(minX, xx);
		minY = MIN(minY, yy);
		maxX = MAX(maxX, xx);
		maxY = MAX(maxY, yy);
	}

    void offsetX(double offset) {
        minX += offset;
        maxX += offset;
    }

    void offsetY(double offset) {
        minY += offset;
        maxY += offset;
    }

    void offset(double xOffset, double yOffset) {
        minX += xOffset;
        maxX += xOffset;
        minY += yOffset;
        maxY += yOffset;
    }

	bool isIntersect(const GeoExtent& rhs) const {
		return minX <= rhs.maxX && maxX >= rhs.minX &&
			minY <= rhs.maxY && maxY >= rhs.minY;
	}

	void intersect(const GeoExtent& rhs) {
		if (isIntersect(rhs)) {
			minX = MAX(minX, rhs.minX);
			minY = MAX(minY, rhs.minY);
			maxX = MIN(maxX, rhs.maxX);
			maxY = MIN(maxY, rhs.maxY);
		}
		else {
			*this = GeoExtent();
		}
	}

	bool contain(const GeoRawPoint& pt) const {
		return pt.x >= minX && pt.x <= maxX 
			&& pt.y >= minY && pt.y <= maxY;
	}

	bool contain(double xx, double yy) const {
		return xx >= minX && xx <= maxX
			&& yy >= minY && yy <= maxY;
	}

	void normalize() {
		if (minX > maxX)
			std::swap(minX, maxX);
		if (minY > maxY)
			std::swap(minY, maxY);
	}

public:
	double minX;
	double maxX;
	double minY;
	double maxY;
};
