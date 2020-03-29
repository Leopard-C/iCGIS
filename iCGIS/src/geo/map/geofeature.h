/*******************************************************************
** class name:  GeoFeature
**
** description: Feature = Geometry + FieldValues
**
** last change: 2020-03-25
********************************************************************/
#pragma once

#include "geo/geometry/geogeometry.h"
#include "geo/map/geofielddefn.h"
#include "util/utility.h"

#include <vector>
#include <QColor>

class GeoFeatureLayer;
class OpenglFeatureDescriptor;

class GeoFeature {
public:
    /* When construct an object  of GeoFeature, the parent layer is required
    ** or pass it the defination of attribute table's header */
    GeoFeature(GeoFeatureLayer* layerParent);
    GeoFeature(int nFID, GeoFeatureLayer* layerParent);
    GeoFeature(std::vector<GeoFieldDefn*>* fieldDefnsIn);
    GeoFeature(int nFID, std::vector<GeoFieldDefn*>* fieldDefnsIn);
    GeoFeature(const GeoFeature& rhs, std::vector<GeoFieldDefn*>* fieldDefnsIn);
    GeoFeature() {}  // shouldn't use this constructor!!! Just for compiling
    ~GeoFeature();

    /********************************************
    **
    **	geometry
    **
    *********************************************/
    void setGeometry(GeoGeometry* geomIn);
    GeoGeometry* getGeometry() const { return geom; }
    GeometryType getGeometryType() const;
    void offset(double xOffset, double yOffset);
    void rotate(double angle);
    void rotate(double sinAngle, double cosAngle);
    void rotate(double centerX, double centerY, double angle);
    void rotate(double centerX, double centerY, double sinAngle, double cosAngle);

    // Boundary
    // The minimum enclosing rectangle
    void updateExtent() { extent = geom->getExtent(); }
    const GeoExtent& getExtent() const { return extent; }


    /********************************************
    **
    **	field
    **
    *********************************************/

    // Init all fields' value before call setField(...)
    void initNewFieldValue();

    // Get field's value
    template<typename T>
    bool getField(QString name, T* outValue) const {
        return getField(getFieldIndexByName(name), outValue);
    }

    template<typename T>
    bool getField(int idx, T* outValue) const {
        *outValue = *(T*)fieldValues[idx];
        return true;
    }

    // Set field's value
    template<typename T>
    void setField(int idx, T valueIn) {
        initNewFieldValue();
        *(T*)(fieldValues[idx]) = valueIn;
    }

    template<typename T>
    void setField(QString name, T valueIn) {
        setField(getFieldIndexByName(name), valueIn);
    }

    int getFID() const { return nFID; }
    void setFID(int nFIDIn) { nFID = nFIDIn; }

    int getNumFields() const { return (*fieldDefns).size(); }
    QString getFieldName(int idx) const;
    GeoFieldType getFieldType(int idx) const;
    GeoFieldType getFieldType(const QString& name) const;

    bool isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int getFieldIndexByName(const QString& name) const;


    /********************************************
    **
    **   Draw to screen
    **
    *********************************************/

    void setOpenglFeatureDescriptor(OpenglFeatureDescriptor* desc);

    void Draw() const;
    void HighlightDraw() const;

    // Selected
    bool isDeleted() const { return deleted; }
    bool isSelected() const { return selected; }
    void setDeleted(bool b) { deleted = b; }
    void setSelected(bool b) { selected = b; }

    // Color (including point's color, line's color, polygon's fill color
    void setColor(unsigned int colorIn, bool bUpdate = false);
    void setColor(int r, int g, int b, bool bUpdate = false);
    void getColorF(float&r, float &g, float& b);

    // Border color, only for polygon features
    void setBorderColor(int colorIn, bool bUpdate = false);
    void setBorderColor(int r, int g, int b, bool bUpdate = false);
    void getBorderColorF(float& r, float& g, float& b);

private:
    bool checkFieldName(const QString& name) const;
    void setColor_(float r, float g, float b);
    void setBorderColor_(float r, float g, float b);

private:
    // Uniquely identify a feature in a layer
    int nFID = 0;

    // Cache the extent to speed up the get-opeartion
    GeoExtent extent;

    // geometry (Only one geom in one feature)
    GeoGeometry* geom = nullptr;

    /* Field defination
    ** There is only one defination in one layer.
    ** And each features in the layer just holds a pointer of the defination */
    std::vector<GeoFieldDefn*>* fieldDefns;

    // Field values
    // Stored as void pointer, get the type from the fieldDefns
    std::vector<void*> fieldValues;

    /* VAO, VBO, IBOs */
    OpenglFeatureDescriptor* openglFeatureDesc = nullptr;

    bool selected = false;
    bool deleted  = false;

    // Point's color, LineString's color, Polygon's fill color
    QRgb color = utils::getRandomColor();   // define QRgb unsigned int

    // Polygon's border color. So this is only used in polygon features
    QRgb borderColor = Qt::black;
};
