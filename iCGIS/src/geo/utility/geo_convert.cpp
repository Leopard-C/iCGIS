#include "geo_convert.h"

#include "util/logger.h"
#include "geo/map/geomap.h"


GeometryType convertOGRwkbGeometryType(OGRwkbGeometryType type)
{
    switch (type) {
    default:					return kGeometryTypeUnknown;
    case wkbPoint:				return kPoint;
    case wkbPoint25D:			return kPoint;
    case wkbLineString:			return kLineString;
    case wkbLinearRing:			return kLinearRing;
    case wkbPolygon:			return kPolygon;
    case wkbMultiPoint:			return kMultiPoint;
    case wkbMultiPolygon:		return kMultiPolygon;
    case wkbMultiLineString:	return kMultiLineString;
    }
}

GeoFieldType convertOGRFieldType(OGRFieldType type)
{
    switch (type) {
    default:			return kFieldUnknown;
    case OFTInteger:	return kFieldInt;
    case OFTReal:		return kFieldDouble;
    case OFTString:		return kFieldText;
    }
}

/***********************************/
/*                                 */
/*       GDALDataset -> GeoMap     */
/*	                               */
/***********************************/
bool convertGDALDataset(GDALDataset* poDsIn, GeoMap* geoMapOut)
{
    if (!poDsIn || !geoMapOut)
        return false;

    int layerCount = poDsIn->GetLayerCount();
    if (layerCount == 0) {
        LInfo("poDsIn dosen't have any layer");
        return true;
    }

    for (int i = 0; i < layerCount; ++i) {
        OGRLayer* poLayer = poDsIn->GetLayer(i);
        GeoFeatureLayer* geoLayer = new GeoFeatureLayer();
        geoLayer->setGeometryType(convertOGRwkbGeometryType(poLayer->GetGeomType()));
        if (convertOGRLayer(poLayer, geoLayer)) {
            geoMapOut->addLayer(geoLayer);
        }
        else {
            delete geoLayer;
            return false;
        }
    }

    return true;
}

/***********************************/
/*                                 */
/*       OGRLayer -> GeoFeatureLayer      */
/*	                               */
/***********************************/
bool convertOGRLayer(OGRLayer* poLayerIn, GeoFeatureLayer* geoLayerOut)
{
    if (!poLayerIn || !geoLayerOut)
        return false;

    poLayerIn->ResetReading();
    //geoLayerOut->setName(QString::fromLocal8Bit(poLayerIn->GetName()));
    geoLayerOut->setName(poLayerIn->GetName());

    //	// Projection info
    //	char* spatialRef = nullptr;
    //	OGRSpatialReference* ref = poLayerIn->GetSpatialRef();
    //	ref->exportToWkt(&spatialRef);
    //	geoLayerOut->setSpatialRef(spatialRef);
    //	CPLFree(spatialRef);
    //
    int featureCount = poLayerIn->GetFeatureCount();
    if (featureCount == 0) {
        LInfo("No features in the layer");
        return true;
    }
    geoLayerOut->reserveFeatureCount(featureCount);

    OGRFeature* poFeature = poLayerIn->GetNextFeature();

    int fieldCount = poFeature->GetFieldCount();
    geoLayerOut->reserveFieldCount(fieldCount);

    // Read header definition of attribute table
    OGRFeatureDefn* poFDefn = poFeature->GetDefnRef();
    OGRFieldDefn* poFieldDefn = nullptr;
    for (int i = 0; i < fieldCount; ++i) {
        poFieldDefn = poFDefn->GetFieldDefn(i);
        GeoFieldDefn* geoFieldDefn = new GeoFieldDefn();

        geoFieldDefn->setName(poFieldDefn->GetNameRef());
        geoFieldDefn->setWidth(poFieldDefn->GetWidth());
        geoFieldDefn->setType(convertOGRFieldType(poFieldDefn->GetType()));

        switch (poFieldDefn->GetType()) {
        default:
            delete geoFieldDefn;
            break;
        case OFTInteger:
            geoLayerOut->addField(geoFieldDefn);
            break;
        case OFTReal:
            geoLayerOut->addField(geoFieldDefn);
            break;
        case OFTString:
            geoLayerOut->addField(geoFieldDefn);
            break;
        }
    } // end for

    unsigned int color = utils::getRandomColor();

    // Read all features
    poLayerIn->ResetReading();
    while (poFeature = poLayerIn->GetNextFeature()) {
        GeoFeature* geoFeature = new GeoFeature(geoLayerOut);
        if (convertOGRFeature(poFeature, geoFeature)) {
            geoFeature->setColor(color, false);    // The features in one layer has the same color firstly
            geoLayerOut->addFeature(geoFeature);
        }
        else
            delete geoFeature;
        OGRFeature::DestroyFeature(poFeature);
    }

    // create spatial index
    geoLayerOut->createGridIndex();

    return true;
}

/*************************************/
/*                                   */
/*       OGRFeature -> GeoFeature    */
/*                                   */
/*************************************/
bool convertOGRFeature(OGRFeature* poFeatureIn, GeoFeature* geoFeatureOut)
{
    if (!poFeatureIn || !geoFeatureOut)
        return false;

    int fieldCount = poFeatureIn->GetFieldCount();
    OGRFieldDefn* poFieldDefn = nullptr;
    OGRFeatureDefn* poFDefn = poFeatureIn->GetDefnRef();

    /* Property Field */
    for (int i = 0, j = 0; i < fieldCount; ++i) {
        poFieldDefn = poFDefn->GetFieldDefn(i);
        switch (poFieldDefn->GetType()) {
        default:
            break;
        case OFTInteger:
            geoFeatureOut->setField(j++, poFeatureIn->GetFieldAsInteger(i));
            break;
        case OFTReal:
            geoFeatureOut->setField(j++, poFeatureIn->GetFieldAsDouble(i));
            break;
        case OFTString:
            if (poFeatureIn->IsFieldNull(i))
                geoFeatureOut->setField(j++, QString(""));
            else
                geoFeatureOut->setField(j++, QString(poFeatureIn->GetFieldAsString(i)));
            //geoFeatureOut->setField(j++, QString::fromLocal8Bit(poFeatureIn->GetFieldAsString(i)));
            break;
        }
    }

    /* Geometry */
    OGRGeometry* poGeometry = poFeatureIn->GetGeometryRef();
    if (!poGeometry)
        return false;

    OGRwkbGeometryType poGeoType = poGeometry->getGeometryType();
    switch (poGeoType) {
    default:
        break;
    case wkbPoint:
    case wkbPoint25D:
    {
        GeoPoint* geoPoint = new GeoPoint();
        if (convertOGRPoint(poGeometry->toPoint(), geoPoint)) {
            geoFeatureOut->setGeometry(geoPoint);
        }
        else {
            delete geoPoint;
            //return false;
        }
        break;
    }
    case wkbPolygon:
    {
        GeoPolygon* geoPolygon = new GeoPolygon();
        if (convertOGRPolygon(poGeometry->toPolygon(), geoPolygon)) {
            geoFeatureOut->setGeometry(geoPolygon);
        }
        else {
            delete geoPolygon;
            return false;
        }
        break;
    }
    case wkbLineString:
    {
        GeoLineString* geoLineString = new GeoLineString();
        if (convertOGRLineString(poGeometry->toLineString(), geoLineString)) {
            geoFeatureOut->setGeometry(geoLineString);
        }
        else {
            delete geoLineString;
            return false;
        }
        break;
    }
    case wkbMultiPoint:
    {
        GeoMultiPoint* geoMultiPoint = new GeoMultiPoint();
        if (convertOGRMultiPoint(poGeometry->toMultiPoint(), geoMultiPoint)) {
            geoFeatureOut->setGeometry(geoMultiPoint);
        }
        else {
            delete geoMultiPoint;
            return false;
        }
        break;
    }
    case wkbMultiPolygon:
    {
        GeoMultiPolygon* geoMultiPolygon = new GeoMultiPolygon();
        if (convertOGRMultiPolygon(poGeometry->toMultiPolygon(), geoMultiPolygon)) {
            geoFeatureOut->setGeometry(geoMultiPolygon);
        }
        else {
            delete geoFeatureOut;
            return false;
        }
        break;
    }
    case wkbMultiLineString:
    {
        GeoMultiLineString* geoMultiLineString = new GeoMultiLineString();
        if (convertOGRMultiLineString(poGeometry->toMultiLineString(), geoMultiLineString)) {
            geoFeatureOut->setGeometry(geoMultiLineString);
        }
        else {
            delete geoMultiLineString;
            return false;
        }
        break;
    }
    } // end swicth

    geoFeatureOut->updateExtent();
    return true;
}

/*************************************/
/*                                   */
/*      OGRPoint -> GeoPint          */
/*                                   */
/*************************************/
bool convertOGRPoint(OGRPoint* poPointIn, GeoPoint* geoPointOut)
{
    if (!poPointIn || !geoPointOut)
        return false;

    geoPointOut->setXY(poPointIn->getX(), poPointIn->getY());

    return true;
}


/*************************************/
/*                                   */
/*      OGRPoint -> GeoPint          */
/*                                   */
/*************************************/
bool convertOGRLineString(OGRLineString* poLineStringIn, GeoLineString* geoLineStringOut)
{
    if (!poLineStringIn || !geoLineStringOut)
        return false;

    int pointsCount = poLineStringIn->getNumPoints();
    geoLineStringOut->reserveNumPoints(pointsCount);
    for (int i = 0; i < pointsCount; ++i) {
        geoLineStringOut->addPoint(poLineStringIn->getX(i), poLineStringIn->getY(i));
    }

    return true;
}


/*************************************/
/*                                   */
/*      OGRPoint -> GeoPint          */
/*                                   */
/*************************************/
bool convertOGRPolygon(OGRPolygon* poPolygonIn, GeoPolygon* geoPolygonOut)
{
    if (!poPolygonIn || !geoPolygonOut)
        return false;

    OGRPoint ptTemp;
    int numInteriorRings = poPolygonIn->getNumInteriorRings();
    geoPolygonOut->reserveInteriorRingsCount(numInteriorRings);

    // Exterial ring
    OGRLinearRing* poExteriorRing = poPolygonIn->getExteriorRing();
    GeoLinearRing* geoExteriorRing = new GeoLinearRing();
    int numExteriorRingPoints = poExteriorRing->getNumPoints();
    geoExteriorRing->reserveNumPoints(numExteriorRingPoints);

    for (int k = 0; k < numExteriorRingPoints; ++k) {
        poExteriorRing->getPoint(k, &ptTemp);
        geoExteriorRing->addPoint(ptTemp.getX(), ptTemp.getY());
    }
    //geoExteriorRing->closeRings();
    geoPolygonOut->setExteriorRing(geoExteriorRing);

    // Interial ring
    for (int i = 0; i < numInteriorRings; ++i) {
        OGRLinearRing* poInteriorRing = poPolygonIn->getInteriorRing(i);
        GeoLinearRing* geoInteriorRing = new GeoLinearRing();
        int numInteriorRingPoints = poInteriorRing->getNumPoints();
        geoInteriorRing->reserveNumPoints(numInteriorRingPoints);
        for (int k = 0; k < numInteriorRingPoints; ++k) {
            poInteriorRing->getPoint(k, &ptTemp);
            geoInteriorRing->addPoint(ptTemp.getX(), ptTemp.getY());
        }
        geoInteriorRing->closeRings();
        geoPolygonOut->addInteriorRing(geoInteriorRing);
    }

    return true;
}


/******************************************/
/*                                        */
/*     OGRMultiPoint -> GeoMultiPoint     */
/*                                        */
/******************************************/
bool convertOGRMultiPoint(OGRMultiPoint* poMultiPointIn, GeoMultiPoint* geoMultiPointOut)
{
    if (!poMultiPointIn || !geoMultiPointOut)
        return false;

    int pointsCount = poMultiPointIn->getNumGeometries();
    for (int i = 0; i < pointsCount; ++i) {
        GeoPoint* geoPoint = new GeoPoint();
        OGRPoint* poPoint = poMultiPointIn->getGeometryRef(i)->toPoint();
        geoPoint->setXY(poPoint->getX(), poPoint->getY());
        geoMultiPointOut->addGeometry(geoPoint);
    }

    return true;
}


/*******************************************/
/*                                         */
/*    OGRMultiPolygon -> GeoMultiPolygon   */
/*                                         */
/*******************************************/
bool convertOGRMultiPolygon(OGRMultiPolygon* poMultipolygonIn, GeoMultiPolygon* geoMultiPolygonOut)
{
    if (!poMultipolygonIn || !geoMultiPolygonOut)
        return false;

    int polygonCount = poMultipolygonIn->getNumGeometries();
    for (int i = 0; i < polygonCount; ++i) {
        GeoPolygon* geoPolygon = new GeoPolygon();
        if (convertOGRPolygon(poMultipolygonIn->getGeometryRef(i)->toPolygon(), geoPolygon))
            geoMultiPolygonOut->addPolygon(geoPolygon);
        else
            delete geoPolygon;
    }

    return true;
}


/*************************************************/
/*                                               */
/*    OGRMultiLineString -> GeoMultiLineString   */
/*                                               */
/*************************************************/
bool convertOGRMultiLineString(OGRMultiLineString* poMultiLineStringIn, GeoMultiLineString* geoMultiLineStringOut)
{
    if (!poMultiLineStringIn || !geoMultiLineStringOut)
        return false;

    int lineStringCount = poMultiLineStringIn->getNumGeometries();
    for (int i = 0; i < lineStringCount; ++i) {
        GeoLineString* geoLineString = new GeoLineString();
        if (convertOGRLineString(poMultiLineStringIn->getGeometryRef(i)->toLineString(), geoLineString))
            geoMultiLineStringOut->addGeometry(geoLineString);
        else
            delete geoLineString;
    }

    return true;
}

