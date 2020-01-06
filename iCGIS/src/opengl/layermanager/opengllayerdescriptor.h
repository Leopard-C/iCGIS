/********************************************************************
** base class:  OpenglLayerDescriptor
**
** sub classes: OpenglFeatureLayerDescriptor
**				OpenglRasterLayerDescriptor
**
** description: 图层描述符，在OpenGLWidget中，向GPU发送图层数据时，
**				会返回一个图层描述符，记录图层的LID和图层中每个要素的
**				要素描述符（即OpenglFeatureDescriptorj
**
** last change: 2020-01-02
*******************************************************************/
#pragma once

#include "opengl/layermanager/openglfeaturedescriptor.h"
#include "opengl/layermanager/openglrasterdescriptor.h"
#include "geo/map/geolayer.h"
#include "utility.h"

class OpenglFeatureLayerDescriptor;
class OpenglRasterLayerDescriptor;



/******************************************/
/*                                        */
/*          图层 描述符 (基类)             */
/*                                        */
/******************************************/
class OpenglLayerDescriptor {
public:
	OpenglLayerDescriptor(int nLID) : LID(nLID) {}
	virtual ~OpenglLayerDescriptor() {}

	OpenglFeatureLayerDescriptor* toFeatureLayerDescriptor()
		{ return utils::down_cast<OpenglFeatureLayerDescriptor*>(this); }
	OpenglRasterLayerDescriptor* toRasterLayerDescriptor()
		{ return utils::down_cast<OpenglRasterLayerDescriptor*>(this); }

	int getLID() const { return LID; }

	virtual LayerType getLayerType() const = 0;

	int LID;
};


/******************************************/
/*                                        */
/*          要素图层 描述符                */
/*                                        */
/******************************************/
class OpenglFeatureLayerDescriptor : public OpenglLayerDescriptor {
public:
	OpenglFeatureLayerDescriptor(int nLID)
		: OpenglLayerDescriptor(nLID) {}
	virtual ~OpenglFeatureLayerDescriptor();

	OpenglFeatureDescriptor* getFeatureDescriptor(int nFID) const;

	void addFeatureDesc(OpenglFeatureDescriptor* featureDesc)
		{ featuresDescriptors.push_back(featureDesc); }

public:
	/* override */
	virtual LayerType getLayerType() const override { return kFeatureLayer; }

public:
	/* data member */
	std::vector<OpenglFeatureDescriptor*> featuresDescriptors;
};


/******************************************/
/*                                        */
/*          栅格图层 描述符                */
/*                                        */
/******************************************/
class OpenglRasterLayerDescriptor : public OpenglLayerDescriptor {
public:
	OpenglRasterLayerDescriptor(int nLID)
		: OpenglLayerDescriptor(nLID) {}
	virtual ~OpenglRasterLayerDescriptor()
		{ if (rasterDesc)  delete rasterDesc; }

public:
	/* override */
	virtual LayerType getLayerType() const override { return kRasterLayer; }

public:
	/* data member */
	OpenglRasterDescriptor* rasterDesc = nullptr;
};
