/*********************************************************************
** class name:  FileReader
**
** last change: 2020-01-02
*********************************************************************/
#pragma once

#include "geo/map/geomap.h"
#include "geo/utility/sld.h"


class FileReader {
public:
	/*********************
	* Vector Layer
	*********************/
	static GeoFeatureLayer* readGeoJsonUsingGDAL(QString filepath, GeoMap* map);

	static GeoFeatureLayer* readGeoJsonMine(QString filepath, GeoMap* map);

	static GeoFeatureLayer* readShapefile(QString filepath, GeoMap* map);


	/*********************
	* Raster Layer
	*********************/
	static GeoRasterLayer* readTiff(QString filepath, GeoMap* map);


	/*********************
	* SLD
	*********************/
	static SLDInfo* readSLD(QString filepath, GeoFeatureLayer* layer);
};
