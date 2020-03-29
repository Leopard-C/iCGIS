/*******************************************************
** class name:  GeoMap
**
** last change: 2020-03-26
*******************************************************/
#pragma once

#include "geo/map/geolayer.h"
#include "geo/map/geomapproperty.h"

#include <map>
#include <deque>
#include <vector>


class GeoMap {
public:
    struct LayerOrder {
        LayerOrder(int nLID, int nOrder)
            : LID(nLID), order(nOrder) {}
        int LID;
        int order;
    };

public:
    GeoMap() {}
    GeoMap(const GeoMap& rhs);
    ~GeoMap();

    // Deep copy
    GeoMap* copy();

    // Draw
    void Draw() const;

    /******************************
    **  GeoLayer
    *****************************/
    bool isEmpty() const { return layers.empty(); }
    int getNumLayers() const { return layers.size(); }
    GeoLayer* getLayerByOrder(int order) const;
    GeoLayer* getLayerById(int idx) const { return layers[idx]; }
    GeoLayer* getLayerByLID(int nLID) const;
    GeoLayer* getLayerByName(const QString& name) const;
    GeoLayer* getLastLayer() const;
    int getLayerLIDByOrder(int order) const
    { return layerOrders[order]; }
    int getLayerIndexByOrder(int order) const;
    int getLayerIndexByName(const QString& name) const;
    int getLayerIndexByLID(int lid) const;

    // Add layer to the map, and return the layer's ID
    int addLayer(GeoLayer* layerIn);
    // Remove layer
    bool removeLayerByLID(int nLID);
    // Move layer(nLID) in front of layer(insertLID)
    void changeLayerOrder(int nLID, int insertLID);

    std::vector<GeoLayer*>::iterator begin() { return layers.begin(); }
    std::vector<GeoLayer*>::iterator end() { return layers.end(); }


    /******************************
    **  Property
    *****************************/
    QString getName() const { return properties.name; }
    GeoExtent getExtent() const { return properties.extent; }

    void setName(const QString& nameIn) { properties.name = nameIn; }
    void updateExtent();

    /************************************************
    **  Spatial query (DO NOT use spatial index)
    ************************************************/
    void queryFeature(double x, double y, double halfEdge,
                      GeoFeatureLayer*& layerOut,
                      GeoFeature*& featureOut);
    void queryFeatures(const GeoExtent& extent,
                       std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& featuresOut);

    /*********************************
    **  Select features
    *********************************/
    void getAllSelectedFeatures(std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& selectedFeatures);
    void emplaceSelectedFeature(int nLID, GeoFeature* sf);
    void emplaceSelectedFeatures(int nLID, const std::vector<GeoFeature*>& sfs);
    void emplaceSelectedFeatures(GeoFeatureLayer* layer, const std::vector<GeoFeature*>& sfs);
    void setSelectedFeatures(const std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>& sfs);
    void clearSelectedFeatures();
    void offsetSelectedFeatures(double xOffset, double yOffset);

private:
    /* The id of the next layer to be added */
    /* Automatically increase */
    int currentLID = 0;

    std::vector<GeoLayer*> layers;

    // Map's prperties
    GeoMapProperty properties;

    // layerOrders[n] is the LID of the layer in order n
    // the top layer's order is 0
    std::deque<int> layerOrders;
};
