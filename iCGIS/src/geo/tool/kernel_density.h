/**************************************************************
** class name:  KernelDensityTool
**
** description: KDE
**
** last change: 2020-01-02
**************************************************************/
#pragma once

#include "geo/tool/geotool.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QObject>


class KernelDensityTool : public GeoTool
{
    Q_OBJECT
public:
    KernelDensityTool(QWidget* parent = nullptr);
    ~KernelDensityTool();

signals:
    void sigSendLayerToGPU(GeoLayer* layer, bool bUpdate = true);
    void sigAddNewLayerToLayersTree(GeoLayer* layer, bool bUpdate = true);

private:
    void setupLayout();
    void initializeFill();
    double getDefaultSearchRadius(GeoFeatureLayer* layer);
    double getDefaultCellSize(GeoFeatureLayer* layer);

public slots:
    void onChangeInputFeatures(const QString& name);
    void onSetOutputRaster();
    void onBtnOKClicked();

private:
    QComboBox* comboInputFeatures;
    QComboBox* comboPopiField;
    QLineEdit* lineEditOutputRaster;
    QLineEdit* lineEditOutputCellSize;
    QLineEdit* lineEditSearchRadius;
    QComboBox* comboAreaUnits;
    QComboBox* comboOutputValuesType;
    QComboBox* comboMethod;
};
