#include "geomap.h"

#include <algorithm>
#include <fstream>

#include "util/utility.h"
#include "util/logger.h"


GeoMap::GeoMap(const GeoMap& rhs)
    : properties(rhs.properties), layerOrders(rhs.layerOrders)
{
    int layersCount = rhs.layers.size();
    layers.reserve(layersCount);
    for (int i = 0; i < layersCount; ++i) {
        layers.push_back(rhs.layers[i]->copy());
    }

}

// deep copy
GeoMap* GeoMap::copy()
{
    return new GeoMap(*this);
}

GeoMap::~GeoMap()
{
    for (auto& layer : layers)
        delete layer;
}

GeoLayer* GeoMap::getLayerByOrder(int order) const
{
    int nLID = layerOrders[order];
    return getLayerByLID(nLID);
}

GeoLayer* GeoMap::getLayerByLID(int nLID) const
{
    int idx = getLayerIndexByLID(nLID);
    if (idx != -1)
        return layers[idx];
    else
        return nullptr;
}

GeoLayer* GeoMap::getLayerByName(const QString& name) const
{
    int idx = getLayerIndexByName(name);
    if (idx != -1)
        return layers[idx];
    else
        return nullptr;
}

// The last layer
GeoLayer* GeoMap::getLastLayer() const
{
    if (layers.empty())
        return nullptr;
    else
        return layers.back();
}

int GeoMap::getLayerIndexByOrder(int order) const
{
    int nLID = layerOrders[order];
    return getLayerIndexByLID(nLID);
}

int GeoMap::getLayerIndexByLID(int nLID) const
{
    int count = layers.size();
    for (int i = 0; i < count; ++i) {
        if (layers[i]->getLID() == nLID)
            return i;
    }
    return -1;
}

int GeoMap::getLayerIndexByName(const QString& name) const
{
    int count = layers.size();
    for (int i = 0; i < count; ++i) {
        if (layers[i]->getName() == name)
            return i;
    }
    return -1;
}

int GeoMap::addLayer(GeoLayer* layerIn)
{
    // Put the newly added layer on top
    layerOrders.push_front(currentLID);
    layerIn->setLID(currentLID++);

    // Get and cache map's extent
    if (isEmpty())
        properties.extent = layerIn->getExtent();
    else
        properties.extent.merge(layerIn->getExtent());
    layers.push_back(layerIn);

    // return the newly added layer's ID (LID)
    return currentLID - 1;
}

// Remove layer by layer's unique ID
bool GeoMap::removeLayerByLID(int nLID)
{
    int idx = getLayerIndexByLID(nLID);
    if (idx != -1) {
        // Remove from layersOrder table
        for (auto iter = layerOrders.begin(); iter != layerOrders.end(); ++iter) {
            if (*iter == nLID) {
                layerOrders.erase(iter);
                break;
            }
        }
        // Remove the layer
        delete layers[idx];
        layers.erase(layers.begin() + idx);
        updateExtent();
        return true;
    }
    else {
        return false;
    }
}

//// Remove layer by layer's name
//bool GeoMap::removeLayerByName(const QString& name)
//{
//    int idx = getLayerIndexByName(name);
//    if (idx != -1) {
//        removeLayer_(idx);
//        return true;
//    }
//    else {
//        return false;
//    }
//}

// Move layer(nLID) in front of layer(insertLID)
void GeoMap::changeLayerOrder(int nLID, int insertLID)
{
    auto iterNLID = std::find(layerOrders.begin(), layerOrders.end(), nLID);
    auto iterInsertLID = std::find(layerOrders.begin(), layerOrders.end(), insertLID);
    // Move forward
    if (iterNLID > iterInsertLID) {
        for (auto iter = iterNLID; iter != iterInsertLID; --iter) {
            *iter = *(iter - 1);
        }
        *iterInsertLID = nLID;
    }
    // Move backward
    else {
        for (auto iter = iterNLID; iter != iterInsertLID - 1; ++iter) {
            *iter = *(iter + 1);
        }
        *(iterInsertLID - 1) = nLID;
    }
}

void GeoMap::updateExtent()
{
    if (isEmpty()) {
        properties.extent = GeoExtent();
    }
    else {
        properties.extent = layers[0]->getExtent();
        int count = layers.size();
        for (int i = 1; i < count; ++i) {
            properties.extent.merge(layers[i]->getExtent());
            // or
            //properties.extent += layers[i]->getExtent();
        }
    }
}


/************************************************
**
**  Draw
**
************************************************/

void GeoMap::Draw() const {
    int layersCount = layers.size();
    for (int i = layersCount - 1; i > -1; --i) {
        auto* layer = getLayerByLID(layerOrders[i]);
        if (layer->isVisible())
            layer->Draw();
    }
}


/************************************************
**
**  Spatial query
**
************************************************/
// Point query
void GeoMap::queryFeature(double x, double y, double halfEdge, GeoFeatureLayer*& layerOut, GeoFeature*& featureOut) {
    int layersCount = layers.size();
    for (int iOrder = 0; iOrder < layersCount; ++iOrder) {
        GeoLayer* layer = getLayerByOrder(iOrder);
        if (layer->getLayerType() == kFeatureLayer) {
            GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
            if (!featureLayer->isVisible())
                continue;
            featureLayer->queryFeatures(x, y, halfEdge, featureOut);
            if (featureOut) {
                layerOut = featureLayer;
                return;
            }
        }
    }
}

// Box query
void GeoMap::queryFeatures(const GeoExtent& extent, std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& featuresOut) {
    int layersCount = layers.size();
    for (int i = 0; i < layersCount; ++i) {
        GeoLayer* layer = getLayerByOrder(i);
        if (layer->getLayerType() == kFeatureLayer) {
            GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
            if (!featureLayer->isVisible())
                continue;
            std::vector<GeoFeature*> features;
            featureLayer->queryFeatures(extent, features);
            if (features.size() > 0) {
                featuresOut.emplace(featureLayer, features);
                //return;
            }
        }
    }
}


/*********************************
**  Select features
*********************************/
void GeoMap::emplaceSelectedFeature(int nLID, GeoFeature *sf) {
    for (auto& layer : layers) {
        if (layer->getLID() == nLID) {
            layer->toFeatureLayer()->emplaceSelectedFeature(sf);
            return;
        }
    }
}

void GeoMap::emplaceSelectedFeatures(int nLID, const std::vector<GeoFeature*>& sfs) {
    for (auto& layer : layers) {
        if (layer->getLID() == nLID) {
            layer->toFeatureLayer()->emplaceSelectedFeatures(sfs);
            return;
        }
    }
}

void GeoMap::emplaceSelectedFeatures(GeoFeatureLayer* layerIn, const std::vector<GeoFeature*>& sfs) {
    int nLID = layerIn->getLID();
    emplaceSelectedFeatures(nLID, sfs);
}

void GeoMap::setSelectedFeatures(const std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& sfs) {
    clearSelectedFeatures();
    for (auto& layer : layers) {
        if (layer->getLayerType() == LayerType::kFeatureLayer) {
            GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
            auto findIt = sfs.find(featureLayer);
            if (findIt != sfs.end()) {
                featureLayer->setSelectedFeatures(findIt->second);
            }
        }
    }
}

void GeoMap::clearSelectedFeatures() {
    for (auto& layer : layers) {
        if (layer->getLayerType() == LayerType::kFeatureLayer) {
            layer->toFeatureLayer()->clearSelectedFeatures();
        }
    }
}


void GeoMap::offsetSelectedFeatures(double xOffset, double yOffset) {
    for (auto& layer : layers) {
        if (layer->getLayerType() == LayerType::kFeatureLayer) {
            layer->toFeatureLayer()->offsetSelectedFeatures(xOffset, yOffset);
        }
    }
}


void GeoMap::getAllSelectedFeatures(std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& selectedFeatures) {
    for (auto& layer : layers) {
        if (layer->getLayerType() == LayerType::kFeatureLayer) {
            GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
            auto& sfs = featureLayer->getSelectedFeatures();
            if (sfs.size() > 0) {
                selectedFeatures.emplace(featureLayer, sfs);
            }
        }
    }
}
