/*****************************************************************
** class name:  ToolBoxTreeWidget
**
** last change: 2020-01-06
*****************************************************************/

#pragma once

#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "geo/map/geomap.h"
#include "geo/tool/kernel_density.h"


class ToolBoxTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    ToolBoxTreeWidget(QWidget* parent = nullptr);
	~ToolBoxTreeWidget();

	void createToolItems();
	
public slots:
	void onDoubleClicked(QTreeWidgetItem* item, int col);

private:
    GeoMap*& map;

    // root item: tool box
	QTreeWidgetItem* toolboxRootItem;
};
