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
** description: 空间数据几何形态
**
** last change: 2020-01-02
**************************************************************/
#pragma once

#include <cassert>
#include <vector>

#include "utility.h"
#include "memoryleakdetect.h"
#include "geo/geo_base.hpp"


/**************** 前置声明类 *****************/
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
	virtual ~GeoGeometry() {}

	virtual GeometryType getGeometryType() const = 0;
	virtual const char* getGeometryName() const = 0;
	virtual int getNumPoints() const = 0;
	virtual GeoExtent getExtent() const = 0;
	virtual bool isEmpty() const = 0;
	virtual void swapXY() = 0;

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
	GeoPoint& operator=(const GeoPoint& rhs) = default;
	~GeoPoint() {}

	double getX() const { return x; }
	double getY() const { return y; }

	void setXY(double xx, double yy) { x = xx; y = yy; }
	void setX(double xx) { x = xx; }
	void setY(double yy) { y = yy; }

	bool operator==(const GeoPoint& rhs)
		{ return x == rhs.x && y == rhs.y; }

/* override */
public:
	GeometryType getGeometryType() const override;
	const char* getGeometryName() const override;
	int getNumPoints() const override;
	GeoExtent getExtent() const override;
	bool isEmpty() const override;
	void swapXY() override;

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
	GeoLineString();
	virtual ~GeoLineString();

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

	double getX(int idx) const { return points[idx].x; }
	double getY(int idx) const { return points[idx].y; }
	void  getPoint(int idx, GeoPoint* point) const;
	void  getRawPoint(int idx, GeoRawPoint* rawPoint) const;
	GeoRawPoint* getRawPoint(int idx) { return &points[idx]; }
	double* getRawData();		// 返回第一个元素的x坐标的首地址

	// 修改点的坐标
	void setPoint(int idx, double xx, double yy);
	void setPoint(int idx, GeoPoint* point);

	// 删除点
	void removePoint(int idx);

	// 添加点
	void addPoint(const GeoRawPoint& rawPoint);
	void addPoint(double xx, double yy);

	// 检查下标合法性
	bool checkIndex(int idx) const;

	// 预分配内存
	void reserveNumPoints(size_t count);

	// 调整points占用的内存空间至合适
	void adjustToFit() { points.shrink_to_fit(); }

/* override */
public:
	virtual GeometryType getGeometryType() const override;
	virtual const char* getGeometryName() const override;
	virtual GeoExtent getExtent() const override;
	virtual int getNumPoints() const override;
	virtual bool isEmpty() const override;
	virtual void swapXY() override;

/* protected 使派生类GeoLinearRing可以访问本类的成员变量 */
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
	virtual ~GeoLinearRing() = default;

	bool isClockwise() const;
	void closeRings();
	inline GeoLineString* toLineString() { return this; }

/* override */
public:
	virtual GeometryType getGeometryType() const override;
	virtual const char* getGeometryName() const override;
};


/**************************************************/
/*                                                */
/*                   GeoPolygon                   */
/*                1个外环 + n个内环                */
/*                                                */
/**************************************************/
class GeoPolygon : public GeoGeometry {
public:
	GeoPolygon() = default;
	virtual ~GeoPolygon();
public:
	void reserveInteriorRingsCount(size_t size);
	int getInteriorRingsCount() const { return rings.size() - 1; }
	GeoLinearRing* getExteriorRing() const;
	GeoLinearRing* getInteriorRing(int idx) const;

	// 获取连续存储所有点坐标的内存区域的指针
	void getRawData(double** rawData) const;

	void setExteriorRing(GeoLinearRing* ring);
	void addInteriorRing(GeoLinearRing* ring);
	void removeInteriorRing(int idx);

	// 检查下标合法性
	bool checkIndex(int idx) const;

	// 调整points占用的内存空间至合适
	void adjustToFit() { rings.shrink_to_fit(); }

/* override */
public:
	virtual GeometryType getGeometryType() const override;
	virtual const char* getGeometryName() const override;
	virtual GeoExtent getExtent() const override;
	virtual int getNumPoints() const override;
	virtual bool isEmpty() const override;
	virtual void swapXY() override;

private:
	// rings[0]: 外环
	// rings[n]: 内环（n > 0)
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
	virtual ~GeoGeometryCollection();

	std::vector<GeoGeometry*>::iterator begin()
		{ return geoms.begin(); }
	std::vector<GeoGeometry*>::iterator end()
		{ return geoms.end(); }

	GeoGeometry* getGeometry(int idx) const { return geoms[idx]; }
	int getNumGeometries() const { return geoms.size(); }

	void addGeometry(GeoGeometry* geom);
	void removeGeometry(int idx);

	bool checkIndex(int idx) const
		{ return idx > -1 && idx < geoms.size(); }

	void reserveNumGeoms(int idx) { geoms.reserve(idx); }
	void adjustToFit() { geoms.shrink_to_fit(); }

/* override */
public:
	virtual GeometryType getGeometryType() const override;
	virtual const char* getGeometryName() const override;
	virtual GeoExtent getExtent() const override;
	virtual int getNumPoints() const override;
	virtual bool isEmpty() const override;
	virtual void swapXY() override;

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
	~GeoMultiPoint() = default;

	GeoPoint* getPoint(int idx) const { return geoms[idx]->toPoint(); }
	void addPoint(GeoPoint* point) { addGeometry(point); }

/* override */
public:
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
	~GeoMultiPolygon() = default;

	GeoPolygon* getPolygon(int idx) const { return geoms[idx]->toPolygon(); }
	int getNumLinearRings() const;
	void addPolygon(GeoPolygon* polygon) { addGeometry(polygon); }

/* override */
public:
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
	virtual ~GeoMultiLineString() = default;

	GeoLineString* getLineString(int idx) const
		{ return geoms[idx]->toLineString(); }
	void addLineString(GeoLineString* lineString)
		{ addGeometry(lineString); }

/* override */
public:
	virtual GeometryType getGeometryType() const override;
	virtual const char* getGeometryName() const override;
};

