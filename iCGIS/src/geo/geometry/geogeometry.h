/*************************************************************
** base class:  GeoGeometry
**
** sub classes: GeoPoint
**              GeoPolygon
**              GeoLineString
**              GeoLinearRing
**              GeoMultiPoint
**              GeoMultiPolygon
**              GeoMultiLineString
**
** last change: 2020-03-26
**************************************************************/
#pragma once

#include <cassert>
#include <vector>

#include "util/utility.h"
#include "util/memoryleakdetect.h"
#include "geo/geo_base.hpp"


/*********** Forward Declaration ************/
class GeoPoint;								//
class GeoPolygon;							//
class GeoLineString;						//
class GeoLinearRing;						//
class GeoMultiPoint;						//
class GeoMultiPolygon;						//
class GeoMultiLineString;					//
/********************************************/

enum GeometryType {
    kPoint					= 0,
    kPolygon				= 1,
    kLineString				= 2,
    kLinearRing				= 3,
    kMultiPoint				= 4,
    kMultiPolygon			= 5,
    kMultiLineString		= 6,
    kGeometryCollection		= 7,
    kGeometryTypeUnknown	= 8
};


/**************************************************/
/*                                                */
/*          GeoGeometry                           */
/*             A base class, pure virtual class   */
/*                                                */
/**************************************************/
class GeoGeometry {
public:
    GeoGeometry() {}
    virtual ~GeoGeometry();

    virtual GeoGeometry* copy() = 0;    // deep copy

    virtual GeometryType getGeometryType() const = 0;
    virtual const char* getGeometryName() const = 0;
    virtual int getNumPoints() const = 0;
    virtual GeoExtent getExtent() const = 0;
    virtual bool isEmpty() const = 0;
    virtual void swapXY() = 0;
    virtual void offset(double xOffset, double yOffset) = 0;
    virtual void rotate(double centerX, double centerY, double sinAngle, double cosAngle) = 0;

    void rotate(double centerX, double centerY, double angle);
    void rotate(double sinAngle, double cosAngle);
    void rotate(double angle);

    /* cast to children class */
    inline GeoPoint* toPoint() { return utils::down_cast<GeoPoint*>(this); }
    inline GeoLineString* toLineString() { return utils::down_cast<GeoLineString*>(this); }
    inline GeoLinearRing* toLinearRing() { return utils::down_cast<GeoLinearRing*>(this); }
    inline GeoPolygon* toPolygon() { return utils::down_cast<GeoPolygon*>(this); }
    inline GeoMultiPoint* toMultiPoint() { return utils::down_cast<GeoMultiPoint*>(this); }
    inline GeoMultiLineString* toMultiLineString() { return utils::down_cast<GeoMultiLineString*>(this); }
    inline GeoMultiPolygon* toMultiPolygon() { return utils::down_cast<GeoMultiPolygon*>(this); }
};


/**************************************************/
/*                                                */
/*                GeoPoint                        */
/*                                                */
/**************************************************/
class GeoPoint : public GeoGeometry {
public:
    GeoPoint();
    GeoPoint(double xx, double yy);
    GeoPoint(const GeoPoint& rhs) = default;
    GeoPoint& operator=(const GeoPoint& rhs) = delete;
    ~GeoPoint() {}

public:
    GeoRawPoint getXY() const { return {x, y}; }
    double getX() const { return x; }
    double getY() const { return y; }

    void setXY(double xx, double yy) { x = xx; y = yy; }
    void setX(double xx) { x = xx; }
    void setY(double yy) { y = yy; }

    bool operator==(const GeoPoint& rhs)
        { return utils::isEqual(x, rhs.x) && utils::isEqual(y, rhs.y); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
    virtual int getNumPoints() const override;
    virtual GeoExtent getExtent() const override;
    virtual bool isEmpty() const override;
    virtual void swapXY() override;
    virtual void offset(double xOffset, double yOffset) override;
    virtual void rotate(double /*centerX*/, double /*centerY*/,
                        double /*sinAngle*/, double /*cosAngle*/) override {}

private:
    double x;
    double y;
};


/**************************************************/
/*                                                */
/*             GeoLineString                      */
/*                                                */
/**************************************************/
class GeoLineString : public GeoGeometry {
public:
    GeoLineString() = default;
    GeoLineString(const GeoLineString& rhs);
    GeoLineString& operator=(const GeoLineString&) = delete;
    virtual ~GeoLineString() = default;

    /* iterator */
public:
    std::vector<GeoRawPoint>::iterator begin()
        { return points.begin(); }
    std::vector<GeoRawPoint>::iterator end()
        { return points.end(); }

public:
    GeoRawPoint& operator[](int idx)
        { return points[idx]; }
    const GeoRawPoint& operator[](int idx) const
        { return points[idx]; }

    GeoRawPoint getXY(int idx) const { return { points[idx].x, points[idx].y }; }
    double getX(int idx) const { return points[idx].x; }
    double getY(int idx) const { return points[idx].y; }
    void  getPoint(int idx, GeoPoint* point) const;
    void  getRawPoint( int idx, GeoRawPoint* rawPoint) const;
    // get the address of the first point's x-coord
    double* getRawData();

    // set point's coordinate
    void setPoint(int idx, double xx, double yy);
    void setPoint(int idx, GeoPoint* point);

    void removePoint(int idx);

    void addPoint(const GeoRawPoint& rawPoint);
    void addPoint(double xx, double yy);

    // reserve memory to store points
    void reserveNumPoints(int count);

    // adjust memory to fit the number of points
    void adjustToFit() { points.shrink_to_fit(); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
    virtual GeoExtent getExtent() const override;
    virtual int getNumPoints() const override;
    virtual bool isEmpty() const override;
    virtual void swapXY() override;
    virtual void offset(double xOffset, double yOffset) override;
    virtual void rotate(double centerX, double centerY, double sinAngle, double cosAngle) override;

protected:
    std::vector<GeoRawPoint> points;
};


/**************************************************/
/*                                                */
/*             GeoLinearRing                      */
/*                                                */
/**************************************************/
class GeoLinearRing : public GeoLineString {
public:
    GeoLinearRing() = default;
    GeoLinearRing(const GeoLinearRing&);
    GeoLinearRing& operator=(const GeoLinearRing&) = delete;
    virtual ~GeoLinearRing() = default;

public:
    bool isClockwise() const;
    void closeRings();
    inline GeoLineString* toLineString() { return this; }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
};


/*****************************************************/
/*                                                   */
/*                   GeoPolygon                      */
/*      1 exterior ring + some interior rings        */
/*                                                   */
/*****************************************************/
class GeoPolygon : public GeoGeometry {
public:
    GeoPolygon() = default;
    GeoPolygon(const GeoPolygon& rhs);
    GeoPolygon& operator=(const GeoPolygon& rhs) = delete;
    virtual ~GeoPolygon();

public:
    void reserveInteriorRingsCount(int size);
    int getInteriorRingsCount() const { return rings.size() - 1; }
    GeoLinearRing* getExteriorRing() const;
    GeoLinearRing* getInteriorRing(int idx) const;

    // get the pointer(address) of the first point's x-coord
    void getRawData(double** rawData) const;

    void setExteriorRing(GeoLinearRing* ring);
    void addInteriorRing(GeoLinearRing* ring);
    void removeInteriorRing(int idx);

    void adjustToFit() { rings.shrink_to_fit(); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
    virtual GeoExtent getExtent() const override;
    virtual int getNumPoints() const override;
    virtual bool isEmpty() const override;
    virtual void swapXY() override;
    virtual void offset(double xOffset, double yOffset) override;
    virtual void rotate(double centerX, double centerY, double sinAngle, double cosAngle) override;

private:
    // rings[0]: exterior ring
    // rings[n]: interior ring(n >= 0)
    std::vector<GeoLinearRing*> rings;
};


/**************************************************/
/*                                                */
/*            GeoGeometryCollection               */
/*                                                */
/**************************************************/
class GeoGeometryCollection : public GeoGeometry {
public:
    GeoGeometryCollection() = default;
    GeoGeometryCollection(const GeoGeometryCollection& rhs);
    GeoGeometryCollection& operator=(const GeoGeometryCollection&) = delete;
    virtual ~GeoGeometryCollection();

public:
    std::vector<GeoGeometry*>::iterator begin()
        { return geoms.begin(); }
    std::vector<GeoGeometry*>::iterator end()
        { return geoms.end(); }

    GeoGeometry* getGeometry(int idx) const { return geoms[idx]; }
    int getNumGeometries() const { return geoms.size(); }

    void addGeometry(GeoGeometry* geom);
    void removeGeometry(int idx);

    void reserveNumGeoms(int idx) { geoms.reserve(idx); }
    void adjustToFit() { geoms.shrink_to_fit(); }

    /* override */
public:
    virtual GeoGeometry* copy() override = 0;
    virtual GeometryType getGeometryType() const override = 0;
    virtual const char* getGeometryName() const override = 0;
    virtual GeoExtent getExtent() const override;
    virtual int getNumPoints() const override;
    virtual bool isEmpty() const override;
    virtual void swapXY() override;
    virtual void offset(double xOffset, double yOffset) override;
    virtual void rotate(double centerX, double centerY, double sinAngle, double cosAngle) override;

protected:
    std::vector<GeoGeometry*> geoms;
};


/**************************************************/
/*                                                */
/*                 GeoMultiPoint                  */
/*                                                */
/**************************************************/
class GeoMultiPoint : public GeoGeometryCollection {
public:
    GeoMultiPoint() = default;
    GeoMultiPoint(const GeoMultiPoint&);
    GeoMultiPoint& operator=(const GeoMultiPoint&) = delete;
    ~GeoMultiPoint() = default;

    GeoPoint* getPoint(int idx) const { return geoms[idx]->toPoint(); }
    void addPoint(GeoPoint* point) { addGeometry(point); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
};


/**************************************************/
/*                                                */
/*             GeoMultiLineString                 */
/*                                                */
/**************************************************/
class GeoMultiLineString : public GeoGeometryCollection {
public:
    GeoMultiLineString() = default;
    GeoMultiLineString(const GeoMultiLineString& rhs);
    GeoMultiLineString& operator=(const GeoMultiLineString&) = delete;
    virtual ~GeoMultiLineString() = default;

public:
    GeoLineString* getLineString(int idx) const
        { return geoms[idx]->toLineString(); }
    void addLineString(GeoLineString* lineString)
        { addGeometry(lineString); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
};


/**************************************************/
/*                                                */
/*                GeoMultiPolygon                 */
/*                                                */
/**************************************************/
class GeoMultiPolygon : public GeoGeometryCollection {
public:
    GeoMultiPolygon() = default;
    GeoMultiPolygon(const GeoMultiPolygon& rhs);
    GeoMultiPolygon& operator=(const GeoMultiPolygon&) = delete;
    ~GeoMultiPolygon() = default;

public:
    GeoPolygon* getPolygon(int idx) const { return geoms[idx]->toPolygon(); }
    int getNumLinearRings() const;
    void addPolygon(GeoPolygon* polygon) { addGeometry(polygon); }

    /* override */
public:
    virtual GeoGeometry* copy() override;
    virtual GeometryType getGeometryType() const override;
    virtual const char* getGeometryName() const override;
};

