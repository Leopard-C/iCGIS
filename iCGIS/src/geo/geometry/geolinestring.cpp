#include "geogeometry.h"


GeoLineString::GeoLineString()
{
}

GeoLineString::~GeoLineString()
{
}

void GeoLineString::getPoint(int idx, GeoPoint* point) const
{
	if (idx < 0 || idx > points.size() || !point)
		return;

	point->setX(points[idx].x);
	point->setY(points[idx].y);
}

void GeoLineString::getRawPoint(int idx, GeoRawPoint* rawPoint) const
{
	if (idx < 0 || idx > points.size() || !rawPoint)
		return;
	rawPoint->x = points[idx].x;
	rawPoint->y = points[idx].y;
}

double* GeoLineString::getRawData()
{
	return &(points[0].x);
}

void GeoLineString::removePoint(int idx)
{
	if (checkIndex(idx))
		points.erase(points.begin() + idx);
}

void GeoLineString::setPoint(int idx, double xx, double yy)
{
	if (checkIndex(idx)) {
		this->points[idx].x = xx;
		this->points[idx].y = yy;
	}
}

void GeoLineString::setPoint(int idx, GeoPoint* point)
{
	setPoint(idx, point->getX(), point->getY());
}

void GeoLineString::addPoint(double xx, double yy)
{
	this->points.emplace_back(xx, yy);
}

void GeoLineString::addPoint(const GeoRawPoint& rawPoint)
{
	this->points.emplace_back(rawPoint);
}

bool GeoLineString::checkIndex(int idx) const
{
	return idx > -1 && idx < points.size();
}

void GeoLineString::reserveNumPoints(size_t count)
{
	this->points.reserve(count);
}


/* Overrider */

GeometryType GeoLineString::getGeometryType() const
{
	return kLineString;
}

const char* GeoLineString::getGeometryName() const
{
	return "LINESTRING";
}

int GeoLineString::getNumPoints() const
{
	return points.size();
}

GeoExtent GeoLineString::getExtent() const
{
	if (isEmpty()) 
		return GeoExtent();

	GeoExtent extentOut(points[0]);

	size_t count = points.size();
	for (int i = 1; i < count; ++i) {
		extentOut.merge(points[i]);
	}

	return extentOut;
}

bool GeoLineString::isEmpty() const
{
	return points.empty();
}

void GeoLineString::swapXY()
{
	size_t count = points.size();
	for (int i = 0; i < count; ++i) {
		std::swap(points[i].x, points[i].y);
	}
}

