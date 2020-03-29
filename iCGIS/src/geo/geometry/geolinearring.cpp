#include "geogeometry.h"


GeoLinearRing::GeoLinearRing(const GeoLinearRing& rhs)
    : GeoLineString(rhs)
{
}

GeoGeometry* GeoLinearRing::copy() {
    return new GeoLinearRing(*this);
}


bool GeoLinearRing::isClockwise() const
{
    // TODO...
    return true;
}

// force to close the ring
void GeoLinearRing::closeRings()
{
    int nPoints = points.size();
    if (nPoints < 2)
        return;

    if (!utils::isEqual(getX(0), getX(nPoints - 1))
        || !utils::isEqual(getY(0), getY(nPoints - 1)))
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
