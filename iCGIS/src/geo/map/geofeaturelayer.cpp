#include "geo/map/geolayer.h"
#include "geo/utility/geo_math.h"

#include <cmath>

GeoLayer::~GeoLayer() {}


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

GeoFeatureLayer::GeoFeatureLayer(const GeoFeatureLayer& rhs) :
    currentFID(rhs.currentFID), properties(rhs.properties)
{
    // Field definitions
    int count = rhs.fieldDefns->size();
    this->fieldDefns = new std::vector<GeoFieldDefn*>;
    this->fieldDefns->reserve(count);
    for (auto& fieldDefn : *(rhs.fieldDefns)) {
        this->fieldDefns->push_back(new GeoFieldDefn(*fieldDefn));
    }

    // copy features
    int featuresCount = rhs.features.size();
    this->features.reserve(featuresCount);
    for (auto& feature : rhs.features) {
        this->features.push_back(new GeoFeature(*feature, this->fieldDefns));
    }

    // grid index
    createGridIndex();
}

// deep copy
GeoLayer* GeoFeatureLayer::copy() {
    return (new GeoFeatureLayer(*this));
}

GeoFeatureLayer::~GeoFeatureLayer()
{
    // Notice the order of destruction

    // Destruct the feature
    for (auto& pFeature : features)
        delete pFeature;

    // and then destruct the field definations
    if (fieldDefns) {
        for (auto& fieldDefn : *fieldDefns) {
            delete fieldDefn;
        }
        delete fieldDefns;
    }

    if (gridIndex)
        delete gridIndex;
}

bool GeoFeatureLayer::addFeature(GeoFeature* feature)
{
    GeometryType typeIn = feature->getGeometryType();
    if (properties.getGeometryType() == kGeometryTypeUnknown) {
        feature->setFID(currentFID++);
        properties.setGeometryType(typeIn);
        // Get and cache the extent
        if (isEmpty())
            properties.extent = feature->getExtent();
        else
            properties.extent.merge(feature->getExtent());
        features.push_back(feature);
        return true;
    }
    else {
        feature->setFID(currentFID++);
        // Get and cache the extent
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

// Fuzzy matching
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
    if (index != -1)
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

// Fuzzy matching
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

// Create grid index
bool GeoFeatureLayer::createGridIndex()
{
    if (isEmpty())
        return false;

    GeoExtent layerExtent = this->getExtent();

    // Grid's size is 3 times the average size of the enclosing rectangle
    //  of all features int the layer
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

    // Adjust the size of grids to make sure the layer's eclosing rectangle
    //  can be divided evenly
    int cols = 64;
    int rows = 64;
    if (gridWidth < 1e-7) {
        for (int i = 1; i <= 64; i *= 2) {
            if (featuresCount < i * i * 16) {
                rows = cols = i;
                break;
            }
        }
        gridWidth = layerExtent.width() / cols;
        gridHeight = layerExtent.height() / rows;
    }
    else {
        cols = int(floor(layerExtent.width() / gridWidth + 0.5));
        rows = int(floor(layerExtent.height() / gridHeight + 0.5));

        // At most 64x64
        const int MAX_NUM = 64;
        cols = cols > MAX_NUM ? MAX_NUM : cols;
        rows = rows > MAX_NUM ? MAX_NUM : rows;
        // At least 1x1
        cols = cols == 0 ? 1 : cols;
        rows = rows == 0 ? 1 : rows;
        gridWidth = layerExtent.width() / cols;
        gridHeight = layerExtent.height() / rows;
    }

    if (gridIndex)
        delete gridIndex;
    gridIndex = new GridIndex();
    gridIndex->reserve(rows * cols);

    // Add grids
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            Grid* grid = new Grid(i * cols + j);
            gridIndex->addGrid(grid);
            GeoExtent gridExtent(layerExtent.minX + j * gridWidth, layerExtent.minX + (j + 1) * gridWidth,
                                 layerExtent.minY + i * gridHeight, layerExtent.minY + (i + 1) * gridHeight);
            grid->setExtent(gridExtent);
            for (int k = 0; k < featuresCount; ++k) {
                GeoFeature* feature = features[k];
                // If the feature's extent dose not intersect the grid,
                //   the feature dose not intersec the grid
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
                    for (int iPoint = 0; iPoint < pointsCount; ++iPoint) {
                        GeoPoint* point = multiPoint->getPoint(iPoint);
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
                    if (gm::isLineStringRectIntersect(lineString, gridExtent)) {
                        grid->addFeature(feature);
                    }
                    break;
                }
                case kMultiLineString:
                {
                    GeoMultiLineString* multiLineString = feature->getGeometry()->toMultiLineString();
                    int linesCount = multiLineString->getNumGeometries();
                    for (int i = 0; i < linesCount; ++i) {
                        if (gm::isLineStringRectIntersect(multiLineString->getGeometry(i)->toLineString(), gridExtent)) {
                            grid->addFeature(feature);
                            break;
                        }
                    }
                    break;
                }
                case kPolygon:
                {
                    GeoPolygon* polygon = feature->getGeometry()->toPolygon();
                    if (gm::isPolygonRectIntersect(polygon, gridExtent)) {
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
                        if (gm::isPolygonRectIntersect(multiPolygon->getGeometry(i)->toPolygon(), gridExtent)) {
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


/*********************************
**
**  Query features
**
*********************************/

void GeoFeatureLayer::queryFeatures(double x, double y, double halfEdge, GeoFeature*& featureResult) const
{
    if (gridIndex && gm::isPointInRect({x, y}, properties.extent)) {
        gridIndex->queryFeature(x, y, halfEdge, featureResult);
    }
}

void GeoFeatureLayer::queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) const
{
    if (gridIndex && gm::isRectIntersect(extent, properties.extent)) {
        gridIndex->queryFeatures(extent, featuresResult);
    }
}


/*********************************
**
**  Draw features
**
*********************************/

void GeoFeatureLayer::Draw() const {
    for (auto& feature : features) {
        if (feature->isSelected() || feature->isDeleted())
            continue;
        feature->Draw();
    }
    for (auto selectedFeature : selectedFetures) {
        selectedFeature->HighlightDraw();
    }
}

/*********************************
**
**  Offset features (move)
**
*********************************/

void GeoFeatureLayer::offsetFeatures(const std::vector<GeoFeature*> &fs, double xOffset, double yOffset) {
    for (auto& feature : fs) {
        feature->offset(xOffset, yOffset);
    }
}

void GeoFeatureLayer::offsetSelectedFeatures(double xOffset, double yOffset) {
    offsetFeatures(this->selectedFetures, xOffset, yOffset);
}


/*********************************
**
**  Select features
**
*********************************/

void GeoFeatureLayer::emplaceSelectedFeature(GeoFeature* sf) {
    selectedFetures.push_back(sf);
    sf->setSelected(true);
}

void GeoFeatureLayer::emplaceSelectedFeature(int nFID) {
    for (auto feature : features) {
        if (feature->getFID() == nFID) {
            selectedFetures.push_back(feature);
            feature->setSelected(true);
            return;
        }
    }
}

void GeoFeatureLayer::emplaceSelectedFeatures(const std::vector<GeoFeature*>& sfs) {
    selectedFetures.insert(selectedFetures.end(), sfs.begin(), sfs.end());
    for (auto& feature : sfs)
        feature->setSelected(true);
}

void GeoFeatureLayer::emplaceSelectedFeatures(const std::vector<int> &nFIDs) {
    for (auto& feature : features) {
        auto findIter = std::find(nFIDs.begin(), nFIDs.end(), feature->getFID());
        if (findIter != nFIDs.end()) {
            feature->setSelected(true);
            selectedFetures.emplace_back(feature);
        }
    }
}

void GeoFeatureLayer::setSelectedFeatures(const std::vector<GeoFeature*>& sfs) {
    for (auto& feature : selectedFetures)
        feature->setSelected(false);
    for (auto& feature : sfs)
        feature->setSelected(true);
    selectedFetures = sfs;
}

void GeoFeatureLayer::setSelectedFeatures(const std::vector<int> &nFIDs) {
    clearSelectedFeatures();
    for (auto& feature : features) {
        auto findIter = std::find(nFIDs.begin(), nFIDs.end(), feature->getFID());
        if (findIter != nFIDs.end()) {
            feature->setSelected(true);
            selectedFetures.emplace_back(feature);
        }
    }
}

void GeoFeatureLayer::clearSelectedFeatures() {
    if (selectedFetures.size() > 0) {
        for (auto& feature : selectedFetures)
            feature->setSelected(false);
        std::vector<GeoFeature*>().swap(selectedFetures);
    }
}


/*****************************************************
**
**  Delete features
**  @softDelete: true ==> just set a delete flag
**               false ==> remove it
**
****************************************************/
void GeoFeatureLayer::deleteFeature(int nFID, bool softDelete/* = true*/) {
    clearSelectedFeatures();
    for (auto iter = features.begin(); iter != features.end(); ++iter) {
        if ((*iter)->getFID() == nFID) {
            if (softDelete)
                (*iter)->setDeleted(true);
            else {
                delete (*iter);
                features.erase(iter);
            }
            break;
        }
    }
}

void GeoFeatureLayer::deleteFeature(GeoFeature* feature, bool softDelete/* = true*/) {
    clearSelectedFeatures();
    for (auto iter = features.begin(); iter != features.end(); ++iter) {
        if ((*iter)->getFID() == feature->getFID()) {
            if (softDelete) {
                feature->setDeleted(true);
            }
            else {
                delete (*iter);
                features.erase(iter);
            }
            return;
        }
    }
}

void GeoFeatureLayer::deleteFeatures(const std::vector<int>& nFIDs, bool softDelete/* = true*/) {
    // soft delete
    if (softDelete) {
        clearSelectedFeatures();
        for (auto iter = features.begin(); iter != features.end(); ) {
            auto findIt = std::find(nFIDs.begin(), nFIDs.end(), (*iter)->getFID());
            if (findIt != nFIDs.end()) {
                (*iter)->setDeleted(true);
            }
        }
    }
    // hard delete
    else {
        // set not-selected flag
        for (auto& feature : selectedFetures)
            feature->setSelected(false);
        // remove
        for (auto iter = features.begin(); iter != features.end(); ) {
            auto findIt = std::find(nFIDs.begin(), nFIDs.end(), (*iter)->getFID());
            if (findIt != nFIDs.end()) {
                delete (*iter);
                iter = features.erase(iter);
            }
            else {
                ++iter;
            }
        }
        // clear selected features
        std::vector<GeoFeature*>().swap(selectedFetures);
    }
}

void GeoFeatureLayer::deleteFeatures(const std::vector<GeoFeature*>& fs, bool softDelete/* = true*/) {
    // soft delete
    if (softDelete) {
        for (auto feature : fs) {
            feature->setDeleted(true);
        }
        clearSelectedFeatures();
    }

    // hard delete
    else {
        // set not-selected flag
        for (auto& feature : selectedFetures)
            feature->setSelected(false);
        // remove
        for (auto iter = features.begin(); iter != features.end(); ) {
            auto findIt = std::find(fs.begin(), fs.end(), *iter);
            if (findIt != fs.end()) {
                delete (*iter);
                iter = features.erase(iter);
            }
            else {
                ++iter;
            }
        }
        // clear selected features
        std::vector<GeoFeature*>().swap(selectedFetures);
    }
}

void GeoFeatureLayer::deleteSelectedFeatures(bool softDelete/* = true*/) {
    deleteFeatures(selectedFetures, softDelete);
}

// clear delete-flags
bool GeoFeatureLayer::clearDeleteFlags(const std::vector<GeoFeature*>& featuresIn) {
    for (auto& feature : featuresIn) {
        feature->setDeleted(false);
    }
    return true;
}

// Clear all delete-flag
bool GeoFeatureLayer::clearAllDeleteFlags() {
    for (auto iter = features.begin(); iter != features.end(); ++iter) {
        if ((*iter)->isDeleted()) {
            (*iter)->setDeleted(false);
        }
    }
    return features.size() > 0;
}

// Remove features which has delete-falg
// return value: true ==> have delete-flags
//               false ==> no delete-flags
bool GeoFeatureLayer::applyAllDeleteFlags() {
    bool flag = false;
    for (auto iter = features.begin(); iter != features.end(); ) {
        if ((*iter)->isDeleted()) {
            delete *iter;
            iter = features.erase(iter);
            flag = true;
        }
        else {
            ++iter;
        }
    }
    return flag;
}

/*********************************
**  Rotate features (move)
*********************************/
void GeoFeatureLayer::rotateSelectedFeatures(double angle) {
    rotateFeatures(selectedFetures, angle);
}

void GeoFeatureLayer::rotateFeatures(const std::vector<GeoFeature*>& fs, double angle) {
    for (auto& feature : fs) {
        feature->rotate(angle);
    }
}
