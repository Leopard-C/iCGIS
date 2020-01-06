#include "geogeometry.h"


// 是否 顺时针
bool GeoLinearRing::isClockwise() const
{
	// TODO...
	return true;
}

// 闭合环
void GeoLinearRing::closeRings()
{
	int nPoints = points.size();
	if (nPoints < 2)
		return;

	if (getX(0) != getX(nPoints - 1) 
		|| getY(0) != getY(nPoints - 1))
	{
		addPoint(getX(0), getY(0));
	}
}


/* Override */

GeometryType GeoLinearRing::getGeometryType() const
{
	return kLinearRing;
}

const char* GeoLinearRing::getGeometryName() const
{
	return "LINEARRING";
}
