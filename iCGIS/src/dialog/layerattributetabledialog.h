/*******************************************************
** class name:  LayerAttributeTableDialog
**
** description: 图层属性表 窗口
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QAction>
#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>

#include "memoryleakdetect.h"
#include "geo/map/geolayer.h"

class OpenGLWidget;


class LayerAttributeTableDialog : public QDialog
{
	Q_OBJECT

public:
	LayerAttributeTableDialog(GeoFeatureLayer* layerIn, OpenGLWidget* openglWidget, QWidget *parent);
	~LayerAttributeTableDialog();

public slots:
	void onSelectRows();

public:
	void createWidgets();
	void createActions();
	void createToolBar();
	void setupLayout();

private:
	static void readAttributeTable(GeoFeatureLayer* layer, QTableWidget* tableWidget);

public:
	GeoFeatureLayer* layer;

	// widgets
	QTableWidget* tableWidget;
	OpenGLWidget* openglWidget;

	// toolBar
	QToolBar* toolBar;

	// actions
	QAction* removeRecorsdAction;
};
