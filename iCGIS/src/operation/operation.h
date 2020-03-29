/*************************************************************
** class name:  Operation
**
** description: Move, Delete, rotate features
**
** last change: 2020-03-26
**************************************************************/
#ifndef OPERATION_H
#define OPERATION_H

#include "geo/geo_base.hpp"
#include "geo/map/geolayer.h"

#include <map>
#include <QObject>

using FeaturesList = std::map<GeoFeatureLayer*, std::vector<GeoFeature*>>;

enum class OperationType {
    Unknown  = -1,
    Delete   = 0,
    Move     = 1,
    Rotate   = 2
};

class Operation : public QObject
{
    Q_OBJECT
public:
    Operation() {}
    virtual ~Operation();

    virtual OperationType getType() = 0;
    virtual void operate() = 0;
    virtual void undo() = 0;

    FeaturesList features;
};

// Delete feature
class OperationDelete : public Operation
{
    Q_OBJECT
signals:
    void sigDeleteFeatures(bool softDelete);
public:
    OperationDelete();
    virtual ~OperationDelete() {}
    virtual OperationType getType() override { return OperationType::Delete; }
    virtual void operate() override;
    virtual void undo() override;
};

// Move feature
class OperationMove : public Operation
{
public:
    OperationMove() {}
    virtual ~OperationMove() {}
    virtual OperationType getType() override { return OperationType::Move; }
    virtual void operate() override;
    virtual void undo() override;
    double xOffset;
    double yOffset;
};

// Rotate feature
class OperationRotate : public Operation
{
    Q_OBJECT
signals:
    void sigSendFeatureToGPU(GeoFeature* feature);

public:
    OperationRotate();
    virtual ~OperationRotate() {}
    virtual OperationType getType() override { return OperationType::Rotate; }
    virtual void operate() override;
    virtual void undo() override;
    // center of rotation
    std::map<GeoFeature*, GeoRawPoint> centrals;
    // rotate angle -180~180
    double angle;
};

#endif // OPERATION_H
