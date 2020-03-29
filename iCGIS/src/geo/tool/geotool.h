/*******************************************************
** class name:  GeoTool
**
** description: Tool class (base class of all tools)
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QDialog>
#include "geo/map/geomap.h"
#include "util/memoryleakdetect.h"


class GeoTool : public QDialog
{
    Q_OBJECT
public:
    GeoTool(QWidget* parent = nullptr);
    virtual ~GeoTool();

protected:
    GeoMap*& map;
};
