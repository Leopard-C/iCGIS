/*****************************************************************
** class name:  ToolBoxTreeWidget
**
** description: 主窗口左下角的工具管理，继承自QTreeWidget
**
** last change:
*****************************************************************/

#pragma once

#include <QAction>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "geo/map/geomap.h"
#include "geo/tool/kernel_density.h"

class LayersTreeWidget;
class OpenGLWidget;


class ToolBoxTreeWidget : public QTreeWidget {
public:
	ToolBoxTreeWidget(GeoMap* mapIn, QWidget* parent = nullptr);
	~ToolBoxTreeWidget();

	void createToolItems();

	void setLayersTreeWidget(LayersTreeWidget* layersTreeWidgetIn)
		{ layersTreeWidget = layersTreeWidgetIn; }
	void setOpenGLWidget(OpenGLWidget* openglWidgetIn)
		{ openglWidget = openglWidgetIn; }
	
public slots:
	void onDoubleClicked(QTreeWidgetItem* item, int col);

private:
	GeoMap* map;

	// tool box 根节点
	QTreeWidgetItem* toolboxRootItem;

	// 其他控件，因为需要经常通信，所以保存一份指针
	// 只有使用权，没有管理权、所有权
	LayersTreeWidget* layersTreeWidget = nullptr;
	OpenGLWidget* openglWidget = nullptr;
};
