#include "geo/map/geolayer.h"
#include "geo/utility/geo_math.h"


GeoFeatureLayer::GeoFeatureLayer()
{
	properties.setGeometryType(kGeometryTypeUnknown);
	fieldDefns = new std::vector<GeoFieldDefn *>;
}

GeoFeatureLayer::GeoFeatureLayer(int nLID)
{
	properties.id = nLID;
	properties.setGeometryType(kGeometryTypeUnknown);
	fieldDefns = new std::vector<GeoFieldDefn *>;
}

GeoFeatureLayer::GeoFeatureLayer(int nLID, GeometryType type)
{
	properties.id = nLID;
	properties.setGeometryType(kGeometryTypeUnknown);
	fieldDefns = new std::vector<GeoFieldDefn *>;
}

GeoFeatureLayer::~GeoFeatureLayer()
{
	// 注意析构的顺序，先析构每个feature
	for (auto& pFeature : features)
		delete pFeature;
	// 再析构字段的定义
	// 因为析构feature是需要字段定义
	if (fieldDefns) {
		for (auto& fieldDefn : *fieldDefns) {
			delete fieldDefn;
		}
		delete fieldDefns;
	}
	// 索引
	if (gridIndex)
		delete gridIndex;
}

bool GeoFeatureLayer::addFeature(GeoFeature* feature)
{
	/* security VS effiency ? */
	GeometryType typeIn = feature->getGeometryType();
	if (properties.getGeometryType() == kGeometryTypeUnknown) {
		feature->setFID(currentFID++);
		properties.setGeometryType(typeIn);
		// 更新边界
		if (isEmpty())
			properties.extent = feature->getExtent();
		else
			properties.extent.merge(feature->getExtent());
		features.push_back(feature);
		return true;
	}
	else {
		feature->setFID(currentFID++);
		// 更新边界
		if (isEmpty())
			properties.extent = feature->getExtent();
		else
			properties.extent.merge(feature->getExtent());
		features.push_back(feature);
		return true;
	}
}

GeoFeature* GeoFeatureLayer::getFeatureByFID(int nFID) const
{
	for (auto& feature : features) {
		if (feature->getFID() == nFID) {
			return feature;
		}
	}
	return nullptr;
}

GeoFieldDefn* GeoFeatureLayer::getFieldDefn(const QString& name) const
{
	int fieldsCount = fieldDefns->size();
	for (int i = 0; i < fieldsCount; ++i) {
		if ((*fieldDefns)[i]->getName() == name)
			return (*fieldDefns)[i];
	}
	return nullptr;
}

int GeoFeatureLayer::getFieldIndex(const QString& name, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int fieldCount = fieldDefns->size();
	for (int iField = 0; iField < fieldCount; ++iField) {
		if ((*fieldDefns)[iField]->getName().compare(name, cs) == 0)
			return iField;
	}
	return -1;
}

int GeoFeatureLayer::getFieldIndexLike(const QString& name, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int fieldCount = fieldDefns->size();
	for (int iField = 0; iField < fieldCount; ++iField) {
		if ((*fieldDefns)[iField]->getName().contains(name, cs) == 0)
			return iField;
	}
	return -1;
}

QStringList GeoFeatureLayer::getFieldList() const
{
	int fieldsCount = fieldDefns->size();
	QStringList fieldsList;
	for (int i = 0; i < fieldsCount; ++i) {
		fieldsList << (*fieldDefns)[i]->getName();
	}
	return fieldsList;
}

int GeoFeatureLayer::addField(GeoFieldDefn* fieldDefnIn)
{
	int index = getFieldIndex(fieldDefnIn->getName());
	if (index != -1) // 已经存在
		return index;
	else {
		fieldDefns->push_back(fieldDefnIn);
		for (auto& feature : features) {
			feature->initNewFieldValue();
		}
		return fieldDefns->size() - 1;
	}
}

int GeoFeatureLayer::addField(const QString& nameIn, int widthIn, GeoFieldType typeIn)
{
	return addField(new GeoFieldDefn(nameIn, widthIn, typeIn));
}

bool GeoFeatureLayer::isFieldExist(GeoFieldDefn* fieldDefnIn)
{
	for (auto& fieldDefn : *fieldDefns) {
		if (*fieldDefn == *fieldDefnIn) {
			return true;
		}
	}
	return false;
}

// 字段是否存在
bool GeoFeatureLayer::isFieldExist(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int count = (*fieldDefns).size();
	for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().compare(fieldName, cs) == 0) {
			return true;
		}
	}
	return false;
}

// 字段是否存在（模糊匹配）
bool GeoFeatureLayer::isFieldExistLike(const QString& fieldName, Qt::CaseSensitivity cs /*= Qt::CaseSensitive*/) const
{
	int count = (*fieldDefns).size();
	for (int i = 0; i < count; ++i) {
		if ((*fieldDefns)[i]->getName().contains(fieldName, cs)) {
			return true;
		}
	}
	return false;
}

void GeoFeatureLayer::updateExtent()
{
	if (features.empty()) {
		properties.extent = GeoExtent();
	}
	else {
		properties.extent = features[0]->getExtent();
		int count = features.size();
		for (int i = 0; i < count; ++i) {
			properties.extent.merge(features[i]->getExtent());
		}
	}
}

// 创建索引
bool GeoFeatureLayer::createGridIndex()
{
	if (isEmpty())
		return false;

	// 图层范围
	GeoExtent layerExtent = this->getExtent();

	// 网格大小取空间要素外包络矩形平均大小的3倍
	double gridWidth = 0;
	double gridHeight = 0;
	int featuresCount = features.size();
	for (int i = 0; i < featuresCount; ++i) {
		const GeoExtent& extent = features[i]->getExtent();
		gridWidth += extent.width();
		gridHeight += extent.height();
	}
	gridWidth = (gridWidth / featuresCount) * 3;
	gridHeight = (gridHeight / featuresCount) * 3;
	// 调整网格大小使其能够均分
	gridWidth = layerExtent.width() / floor(layerExtent.width() / gridWidth + 0.5);
	gridHeight = layerExtent.height() / floor(layerExtent.height() / gridHeight + 0.5);

	// 网格行列数
	int row = floor(layerExtent.height() / gridHeight + 0.5);
	int col = floor(layerExtent.width() / gridWidth + 0.5);

	if (gridIndex)
		delete gridIndex;
	gridIndex = new GridIndex();
	gridIndex->reserve(row * col);

	// 添加网格
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			Grid* grid = new Grid(i * col + j);
			gridIndex->addGrid(grid);
			GeoExtent gridExtent(layerExtent.minX + j * gridWidth, layerExtent.minX + (j + 1) * gridWidth,
				layerExtent.minY + i * gridHeight, layerExtent.minY + (i + 1) * gridHeight);
			grid->setExtent(gridExtent);
			// 遍历每个feature
			for (int k = 0; k < featuresCount; ++k) {
				GeoFeature* feature = features[k];
				// 先简单判断一下feature的边界是否和该网格相交
				// 如果不相交，则该feature就不会和该网格相交
				if (!gridExtent.isIntersect(feature->getExtent())) {
					continue;
				}
				switch (feature->getGeometryType()) {
				default:
					break;
				case kPoint:
				{
					GeoPoint* point = feature->getGeometry()->toPoint();
					if (gridExtent.contain(point->getX(), point->getY())) {
						grid->addFeature(feature);
					}
					break;
				}
				case kMultiPoint:
				{
					GeoMultiPoint* multiPoint = feature->getGeometry()->toMultiPoint();
					int pointsCount = multiPoint->getNumGeometries();
					GeoPoint* point;
					for (int iPoint = 0; iPoint < pointsCount; ++iPoint) {
						point = multiPoint->getPoint(iPoint);
						if (gridExtent.contain(point->getX(), point->getY())) {
							grid->addFeature(feature);
							break;
						}
					}
					break;
				}
				case kLineString:
				{
					GeoLineString* lineString = feature->getGeometry()->toLineString();
					if (isLineStringRectIntersect(lineString, gridExtent)) {
						grid->addFeature(feature);
					}
					break;
				}
				case kMultiLineString:
				{
					GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
					int linesCount = multiLineString->getNumGeometries();
					for (int i = 0; i < linesCount; ++i) {
						if (isLineStringRectIntersect(multiLineString->getGeometry(i)->toLineString(), gridExtent)) {
							grid->addFeature(feature);
							break;
						}
					}
					break;
				}
				case kPolygon:
				{
					GeoPolygon* polygon = feature->getGeometry()->toPolygon();
					if (isPolygonRectIntersect(polygon, gridExtent)) {
						grid->addFeature(feature);
					}
					break;
				}
				case kMultiPolygon:
				{
					GeoMultiPolygon* multiPolygon = feature->getGeometry()->toMultiPolygon();
					int polygonsCount = multiPolygon->getNumGeometries();
					for (int i = 0; i < polygonsCount; ++i) {
						//GeoExtent extent = multiPolygon->getGeometry(i)->toPolygon()->getExtent();
						if (isPolygonRectIntersect(multiPolygon->getGeometry(i)->toPolygon(), gridExtent)) {
							grid->addFeature(feature);
							break;
						}
					}
					break;
				}
				} // end switch geometry type
			} // end for k
		} // end for j
	} // end for i


	return true;
}

void GeoFeatureLayer::queryFeatures(double x, double y, GeoFeature*& featureResult) const
{
	if (gridIndex) {
		gridIndex->queryFeatures(x, y, featureResult);
	}
}

void GeoFeatureLayer::queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) const
{
	if (gridIndex) {
		gridIndex->queryFeatures(extent, featuresResult);
	}
}

