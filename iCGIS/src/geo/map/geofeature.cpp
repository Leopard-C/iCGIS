#include "opengl/glcall.h"
#include "geo/map/geofeature.h"
#include "geo/map/geolayer.h"
#include "opengl/openglfeaturedescriptor.h"
#include "util/env.h"


GeoFeature::GeoFeature(GeoFeatureLayer* layerParent) :
	fieldDefns(layerParent->getFieldDefns())
{
	initNewFieldValue();
}


GeoFeature::GeoFeature(std::vector<GeoFieldDefn*>* fieldDefnsIn) :
	fieldDefns(fieldDefnsIn)
{
	initNewFieldValue();
}

GeoFeature::GeoFeature(int nFID, GeoFeatureLayer* layerParent) :
	nFID(nFID), fieldDefns(layerParent->getFieldDefns())
{
	initNewFieldValue();
}

GeoFeature::GeoFeature(int nFID, std::vector<GeoFieldDefn*>* fieldDefnsIn) :
	nFID(nFID), fieldDefns(fieldDefnsIn)
{
	initNewFieldValue();
}

// Copy construct, deep copy
GeoFeature::GeoFeature(const GeoFeature& rhs, std::vector<GeoFieldDefn*>* fieldDefnsIn/* = nullptr*/) :
    nFID(rhs.nFID), extent(rhs.extent), color(rhs.color), borderColor(rhs.borderColor)
{
    this->geom = rhs.geom->copy();

    // Field definitions
    if (fieldDefnsIn == nullptr)
        this->fieldDefns = rhs.fieldDefns;
    else
        this->fieldDefns = fieldDefnsIn;

    // Attention!!!
    initNewFieldValue();

    // Field values
    //std::vector<void*> fieldValues;
    int count = fieldDefns->size();
    for (int i = 0; i < count; ++i) {
        GeoFieldType fieldType = (*fieldDefns)[i]->getType();
        switch (fieldType) {
        case kFieldInt: {
            int value;
            rhs.getField(i, &value);
            this->setField(i, value);
            break;
        }
        case kFieldDouble: {
            double value;
            rhs.getField(i, &value);
            this->setField(i, value);
            break;
        }
        case kFieldText: {
            QString value;
            rhs.getField(i, &value);
            this->setField(i, value);
            break;
        }
        case kFieldUnknown:
            break;
        }
    }
}

GeoFeature::~GeoFeature()
{
    if (geom)
        delete geom;

    if (openglFeatureDesc)
        delete openglFeatureDesc;

    int count = fieldDefns->size();
    for (int i = 0; i < count; ++i) {
		GeoFieldType fieldType = (*fieldDefns)[i]->getType();
		switch (fieldType) {
		case kFieldInt:
			delete (int*)fieldValues[i];
			break;
		case kFieldDouble:
			delete (double*)fieldValues[i];
			break;
		case kFieldText:
			delete (QString*)fieldValues[i];
			break;
		default:
			break;
		}
	}
}


void GeoFeature::setGeometry(GeoGeometry* geomIn)
{
	if (this->geom)
		delete this->geom;
	this->geom = geomIn;
}

GeometryType GeoFeature::getGeometryType() const
{
	if (!geom)
		return kGeometryTypeUnknown;
	else
		return geom->getGeometryType();
}

void GeoFeature::offset(double xOffset, double yOffset) {
    geom->offset(xOffset, yOffset);
    extent.offset(xOffset, yOffset);
    openglFeatureDesc->offset(xOffset, yOffset);
}

void GeoFeature::rotate(double angle) {
    geom->rotate(angle);
    updateExtent();
}

void GeoFeature::rotate(double sinAngle, double cosAngle) {
    geom->rotate(sinAngle, cosAngle);
    updateExtent();
}

void GeoFeature::rotate(double centerX, double centerY, double angle) {
    geom->rotate(centerX, centerY, angle);
    updateExtent();
}

void GeoFeature::rotate(double centerX, double centerY, double sinAngle, double cosAngle) {
    geom->rotate(centerX, centerY, sinAngle, cosAngle);
    updateExtent();
}

/********************************************************
 *
 *  Fields
 *
********************************************************/

void GeoFeature::initNewFieldValue()
{
    int numFieldDefns = fieldDefns->size();
    int numFieldValues = fieldValues.size();
    for (int i = numFieldValues; i < numFieldDefns; ++i) {
		switch ((*fieldDefns)[i]->getType()) {
		default: 
			break;
		case kFieldInt:
			fieldValues.push_back(new int(0));
			break;
		case kFieldDouble:
			fieldValues.push_back(new double(0.0));
			break;
		case kFieldText:
			fieldValues.push_back(new QString());
			break;
		}
	}
}

QString GeoFeature::getFieldName(int idx) const
{
	return (*fieldDefns)[idx]->getName();
}

GeoFieldType GeoFeature::getFieldType(int idx) const
{
	return (*fieldDefns)[idx]->getType();
}

GeoFieldType GeoFeature::getFieldType(const QString& name) const
{
	int idx = getFieldIndexByName(name);
	if (idx != -1)
		return getFieldType(idx);
	else
		return kFieldUnknown;
}

bool GeoFeature::checkFieldName(const QString& name) const
{
	return getFieldIndexByName(name) != -1;
}

/* Fuzzy match field's anem */
bool GeoFeature::isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
    int count = (*fieldDefns).size();
    for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().compare(fieldName, cs) == 0) {
			return true;
		}
	}
	return false;
}


/* Fuzzy match */
bool GeoFeature::isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
    int count = (*fieldDefns).size();
    for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().contains(fieldName, cs)) {
			return true;
		}
	}
	return false;
}

int GeoFeature::getFieldIndexByName(const QString& name) const
{
    int count = (*fieldDefns).size();
    for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName() == name) {
			return i;
		}
	}
	return -1;
}


/************************************************************
 *
 *   Draw
 *
********* ***************************************************/

void GeoFeature::setOpenglFeatureDescriptor(OpenglFeatureDescriptor *desc) {
    if (openglFeatureDesc)
        delete openglFeatureDesc;
    openglFeatureDesc = desc;
}

void GeoFeature::Draw() const {
    switch (geom->getGeometryType()) {
    case kPoint:
    case kMultiPoint: {
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawPoint(openglFeatureDesc->vao, ibo, Env::pointShader);
        }
        break;
    }
    case kLineString:
    case kMultiLineString: {
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawLine(openglFeatureDesc->vao, ibo, Env::lineShader);
        }
        break;
    }
    case kPolygon:
    case kMultiPolygon: {
        /* Fill Color */
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawPolygon(openglFeatureDesc->vao, ibo, Env::polygonShader);
        }
        /* Draw border */
        Env::renderer.DrawPolygonBorder(openglFeatureDesc->vao, Env::borderShader);
        break;
    }
    default:
        break;
    }
}

void GeoFeature::HighlightDraw() const {
    switch (geom->getGeometryType()) {
    case kPoint:
    case kMultiPoint: {
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawPoint(openglFeatureDesc->vao, ibo, Env::highlightShader);
        }
        break;
    }
    case kLineString:
    case kMultiLineString: {
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawLine(openglFeatureDesc->vao, ibo, Env::highlightShader);
        }
        break;
    }
    case kPolygon:
    case kMultiPolygon: {
        /* Fill color */
        for (const auto& ibo : openglFeatureDesc->ibos) {
            Env::renderer.DrawPolygon(openglFeatureDesc->vao, ibo, Env::polygonShader);
        }
        /* Draw border */
        //Env::renderer.DrawHighlight(openglFeatureDesc->vao, Env::highlightShader);
        Env::renderer.DrawPolygonBorder(openglFeatureDesc->vao, Env::highlightShader);
        break;
    }
    default:
        break;
    }
}

void GeoFeature::setColor_(float r, float g, float b) {
    int count = openglFeatureDesc->vbo->getSize() / sizeof(float);
    openglFeatureDesc->vbo->Bind();
    GLCall(float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    /* x, y, r, g, b */
    int stride = openglFeatureDesc->stride;
    for (int i = 0; i < count; i += stride) {
        // data[i] = data[i];				// x
        // data[i + 1] = data[i + 1];		// y
        data[i + 2] = r;			// r
        data[i + 3] = g;			// g
        data[i + 4] = b;			// b
    }

    GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}

void GeoFeature::setColor(unsigned int colorIn, bool bUpdate) {
    color = colorIn;
    if (bUpdate) {
        float rF = qRed(colorIn) / 255.0f;
        float gF = qGreen(colorIn) / 255.0f;
        float bF = qBlue(colorIn) / 255.0f;
        setColor_(rF, gF, bF);
    }
}

void GeoFeature::setColor(int r, int g, int b, bool bUpdate) {
    color = qRgb(r, g, b);
    if (bUpdate) {
        setColor_(r / 255.0f, g / 255.0f, b / 255.0f);
    }
}

void GeoFeature::getColorF(float &r, float &g, float &b) {
    r = qRed(color)   / 255.0f;    // (color >> 16) & 0xff;
    g = qGreen(color) / 255.0f;  // (color >> 8) & 0xff;
    b = qBlue(color)  / 255.0f;   // color & 0xff;
    //a = qAlpha(color) / 255.0f;  // color >> 24;
}


/***********************************
 *
 *  Border Color
 *     Only for polygons
 *
**********************************/

void GeoFeature::setBorderColor_(float r, float g, float b) {
    int count = openglFeatureDesc->vbo->getSize() / sizeof(float);
    openglFeatureDesc->vbo->Bind();
    GLCall(float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    /* x, y, r, g, b, r, g, b */
    int stride = openglFeatureDesc->stride; // 8
    for (int i = 0; i < count; i += stride) {
        // data[i] = data[i];				// x
        // data[i + 1] = data[i + 1];		// y
        // data[i + 2] = r;			// fill color r
        // data[i + 3] = g;			// fill color g
        // data[i + 4] = b;			// foll color b
        data[i + 5] = r;			// border color r
        data[i + 6] = g;			// border color g
        data[i + 7] = b;			// border color b
    }

    GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}

void GeoFeature::setBorderColor(int colorIn, bool bUpdate) {
    borderColor = colorIn;
    if (bUpdate) {
        float rF = qRed(colorIn) / 255.0f;
        float gF = qGreen(colorIn) / 255.0f;
        float bF = qBlue(colorIn) / 255.0f;
        setBorderColor_(rF, gF, bF);
    }
}

void GeoFeature::setBorderColor(int r, int g, int b, bool bUpdate) {
    borderColor = qRgb(r, g, b);
    if (bUpdate) {
        setBorderColor_(r / 255.0f, g / 255.0f, b / 255.0f);
    }
}
