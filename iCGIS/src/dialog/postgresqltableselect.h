/*****************************************************************
** class name:  PostgresqlTableSelect
**
** description: 连接数据库，显示本对话框，用户勾选要导入的数据表
**				每个数据表对应一个图层，可以一次导入多个数据表
**
** last change: 2020-01-02
*****************************************************************/
#pragma once

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <gdal/ogr_geometry.h>
#include <gdal/ogrsf_frmts.h>

#include "geo/map/geomap.h"
#include "geo/map/geofeature.h"

class LayersTreeWidget;
class OpenGLWidget;


class PostgresqlTableSelect : public QDialog
{
	Q_OBJECT
public:
	PostgresqlTableSelect(QWidget *parent);
	~PostgresqlTableSelect();

public:
	void setupLayout();
	bool parseWkbPolygon(OGRPolygon* poPolygon, GeoFeature* geoFeature);
	bool parseWkbMultipoint(OGRMultiPoint* poMultiPoint, GeoFeature* geoFeature);
	bool parseWkbMultipolygon(OGRMultiPolygon* poMultipolygon, GeoFeature* getFeature);

	void setLayersTreeWidget(LayersTreeWidget* layersTreeWidgetIn)
		{ layersTreeWidget = layersTreeWidgetIn; }
	void setOpenglWidget(OpenGLWidget* openglWidgetIn)
		{ openglWidget = openglWidgetIn; }

signals:
	void updateCheckState(Qt::CheckState checkState);

public slots:
	void onConnectPostgresql(const QString& ip, int port, const QString& username,
		const QString& password, const QString& database);
	void onImportLayers();
	void onCancel();
	void updateCheckStateFromHeader(Qt::CheckState checkState);
	void cellCheckboxChanged();

public:
	// layout
	QVBoxLayout* verticalLayout;	// main layout
	QHBoxLayout* horizontalLayout;

	// table widget
	QTableWidget* layersList;
	LayersTreeWidget* layersTreeWidget;
	OpenGLWidget* openglWidget;
	
	// spacerItem
	QSpacerItem* horizontalSpacer_1;
	QSpacerItem* horizontalSpacer_2;
	QSpacerItem* horizontalSpacer_3;
	
	// push button
	QPushButton* btnImportLayers;
	QPushButton* btnCancel;

	GDALDataset* poDS = nullptr;
	GeoMap* map = nullptr;
};
