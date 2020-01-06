#include "geo/utility/filereader.h"

#include "geo/utility/geojson.h"
#include "geo/utility/geo_convert.h"
#include "geo/utility/sld.h" 
#include "geo/raster/geotiff.h"
#include "logger.h"

#include <gdal/gdal_frmts.h>

/*************************************************/
/*                                               */
/*          Read GeoJson (Using GDAL)            */ 
/*                                               */
/*************************************************/
GeoFeatureLayer* FileReader::readGeoJsonUsingGDAL(QString filepath, GeoMap* map)
{
	QByteArray bytes = filepath.toLocal8Bit();
	const char* path = bytes.data();
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO"); //支持中文路径
	CPLSetConfigOption("SHAPE_ENCODING", ""); //解决中文乱码问题
	GDALDataset* poDS = nullptr;
	poDS = (GDALDataset*)GDALOpenEx(path, GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS) {
		LError("Open geojson:{0} error", path);
		return nullptr;
	}

	// 读取GDALDataset到GeoMap
	if (!convertGDALDataset(poDS, map)) {
		LError("Read geojson:{0} error", path);
		GDALClose(poDS);
		return nullptr;
	}

	GDALClose(poDS);

	return (*(map->end() - 1))->toFeatureLayer();
}


/*************************************************/
/*                                               */
/*          Read GeoJson (My own method)         */ 
/*                                               */
/*************************************************/
GeoFeatureLayer* FileReader::readGeoJsonMine(QString filepath, GeoMap* map)
{
	QByteArray bytes = filepath.toLocal8Bit();
	const char* path = bytes.data();
	GeoJson geoJson;
	GeoFeatureLayer* layer = new GeoFeatureLayer();
	if (geoJson.parse(path, layer)) {
		layer->setName(utils::getFileName(filepath));
		map->addLayer(layer);
		return layer;
	}
	else {
		LError("Read geojson:{0} error", path);
		delete layer;
		return nullptr;
	}
}


/*************************************************/
/*                                               */
/*          Read Shapefile (Using GDAL)          */ 
/*                                               */
/*************************************************/
GeoFeatureLayer* FileReader::readShapefile(QString filepath, GeoMap* map)
{
	QByteArray bytes = filepath.toLocal8Bit();
	const char* path = bytes.data();
	GDALAllRegister();
	GDALDataset* poDS = nullptr;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO"); //支持中文路径
	CPLSetConfigOption("SHAPE_ENCODING", ""); //解决中文乱码问题
	poDS = (GDALDataset*)GDALOpenEx(path, GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS) {
		LError("Open geojson:{0} error", path);
		return nullptr;
	}

	GeoFeatureLayer* geoLayer = nullptr;
	if (!convertGDALDataset(poDS, map)) {
		LError("Read shapefile:{0} error", path);
		GDALClose(poDS);
		return nullptr;
	}

	GDALClose(poDS);
	return (*(map->end() - 1))->toFeatureLayer();
}



/*************************************************/
/*                                               */
/*          Read Tiff image (Using GDAL)         */ 
/*                                               */
/*************************************************/
GeoRasterLayer* FileReader::readTiff(QString filepath, GeoMap* map)
{
	QByteArray bytes = filepath.toLocal8Bit();
	const char* path = bytes.data();
	GDALAllRegister();
	GDALDataset* poDS = nullptr;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO"); //支持中文路径
	CPLSetConfigOption("SHAPE_ENCODING", ""); //解决中文乱码问题
	poDS = (GDALDataset*)GDALOpenEx(path, GDAL_OF_RASTER, NULL, NULL, NULL);
	if (!poDS) {
		LError("Open TIFF:{0} error", path);
		return nullptr;
	}

	int width = poDS->GetRasterXSize();
	int height = poDS->GetRasterYSize();
	int bandNum = poDS->GetRasterCount();

	if (bandNum < 1) {
		GDALClose(poDS);
		return nullptr;
	}

	GeoRasterLayer* rasterLayer = new GeoRasterLayer();
	rasterLayer->setName(utils::getFileName(filepath));
	GeoTiff* tiff = new GeoTiff();
	tiff->setBandsCount(bandNum);

	for (int iBand = 0; iBand < bandNum; ++iBand) {
		GDALRasterBand* gdalBand = poDS->GetRasterBand(iBand + 1);	// 第一个波段从1开始
		GeoRasterBand* geoBand = new GeoRasterBand(width, height);

		// 获取仿射变换的参数
		poDS->GetGeoTransform(geoBand->geoTransform);

		GDALDataType dataType = poDS->GetRasterBand(iBand + 1)->GetRasterDataType();

		switch (dataType) {
		default:
			break;
		case GDT_Float32:
			float* pDataBuf = new float[width * height];
			gdalBand->RasterIO(GF_Read, 0, 0, width, height, pDataBuf, width, height, GDT_Float32, 0, 0);
			geoBand->setData(pDataBuf, utils::DataType::kFloat);
			break;
		}

		tiff->setBand(iBand, geoBand);
	}

	rasterLayer->setData(tiff);
	map->addLayer(rasterLayer);

	GDALClose(poDS);
	return rasterLayer;
}


/*************************************************/
/*                                               */
/*              Read SLD                         */ 
/*                                               */
/*************************************************/
SLDInfo* FileReader::readSLD(QString filepath, GeoFeatureLayer* layer)
{
	SLD sld(layer);
	SLDInfo* sldInfo = nullptr;
	if (sld.read(filepath, &sldInfo))
		return sldInfo;
	else
		return nullptr;
}

