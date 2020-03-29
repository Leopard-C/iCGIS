#ifndef APPEVENT_H
#define APPEVENT_H

#include <QObject>

class GeoFeatureLayer;
class GeoRasterLayer;
class GeoLayer;
class GeoFeature;


class AppEvent : public QObject
{
    Q_OBJECT
public:
    static AppEvent* getInstance();

signals:
    void sigNewMap(const QString& name, const QString& path);
    void sigUpdateOpengl();
    void sigAddNewLayerToLayersTree(GeoLayer* layer);
    void sigSendMapToGPU(bool bUpdate = true);
    void sigSendLayerToGPU(GeoLayer* layer, bool bUpdate = true);
    void sigSendFeatureToGPU(GeoFeature* feature);
    void sigZoomToMap();
    void sigZoomToLayer(GeoLayer* layer);
    void sigUpdateCoord(double geoX, double geoY);
    void sigStartEditing(bool on);
    void sigUpdateCursorType();
    void sigDeleteFeatures(bool softDelete);

public slots:
    void onNewMap(const QString& name, const QString& path);
    void onUpdateOpengl();
    void onAddNewLayerToLayersTree(GeoLayer* layer);
    void onSendMapToGPU(bool bUpdate = true);
    void onSendLayerToGPU(GeoLayer* layer, bool bUpdate = true);
    void onSendFeatureToGPU(GeoFeature* feature);
    void onZoomToMap();
    void onZoomToLayer(GeoLayer* layer);
    void onUpdateCoord(double geoX, double geoY);
    void onStartEditing(bool on);
    void onUpdateCursorType();
    void onDeleteFeatures(bool softDelete);

private:
    explicit AppEvent(QObject *parent = nullptr);
    static AppEvent* pInstance;
};

#endif // APPEVENT_H
