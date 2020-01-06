/*******************************************************
** class name:  GeoTool
**
** description: 工具类，所有工具都继承自本类
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QDialog>
#include "geo/map/geomap.h"
#include "memoryleakdetect.h"


class GeoTool : public QDialog {
	Q_OBJECT
public:
	GeoTool(GeoMap* mapIn, QWidget* parent = nullptr)
		: QDialog(parent), map(mapIn) {}
	virtual ~GeoTool() {}

	// 运行工具
	//virtual void run() = 0;

protected:
	GeoMap* map;
};
