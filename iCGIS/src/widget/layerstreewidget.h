/************************************************************
** class name:  LayersTreeWidget
**
** description: 主窗口左上角的图层管理，继承自QTreeWidget
**				支持：图层重命名、删除、拖动调整顺序、
**					打开属性表、自定义样式、开启/关闭编辑
**
** last change: 2020-01-02
************************************************************/

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>

#include <map>

#include "geo/map/geomap.h"
#include "dialog/layerattributetabledialog.h"
#include "dialog/layerstyledialog.h"
#include "./layerstreewidgetitem.h"

class OpenGLWidget;

class LayersTreeWidget : public QTreeWidget {
	Q_OBJECT
public:
	LayersTreeWidget(GeoMap* mapIn, QWidget* parent = nullptr);
	~LayersTreeWidget();

public slots:
	void updateLayersTree();
	void onItemChanged(QTreeWidgetItem* item, int column);
	void onRemoveLayer();
	void onRenameItem();
	void onOpenAttributeTable();
	void onShowStyleDialog();
	void onStartEditing();
	void onStopEditing();
	void onZoomToLayer();

public:
	void createDialogs();
	void createActions();
	void createMenus();
	void insertNewItem(GeoLayer* newLayer);
	void setOpenGLWidget(OpenGLWidget* openglWidgetIn)
		{ openglWidget = openglWidgetIn; }

protected:
	//void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void startDrag(Qt::DropActions supportedActions);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void contextMenuEvent(QContextMenuEvent *event) override;
	void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

public:
	// dialog
	LayerAttributeTableDialog* attributeTableDialog = nullptr;
	LayerStyleDialog* styleDialog = nullptr;

private:
	LayersTreeWidgetItem* toLayerItem(QTreeWidgetItem* item)
		{ return dynamic_cast<LayersTreeWidgetItem*>(item); }

	GeoMap* map;
	OpenGLWidget* openglWidget;

	// item
	QTreeWidgetItem* rootItem; // 根节点，地图名称
	LayersTreeWidgetItem* rightClickedItem;

	// actions
	QAction* removeLayerAction;
	QAction* renameItemAction;
	QAction* zoomToLayerAction;
	QAction* openAttributeTableAction;
	QAction* startEditingAction;
	QAction* stopEditingAction;
	QAction* showStyleDialog;

	// menus
	QMenu* popMenuOnFeatureLayer;	// 要素图层节点 右键菜单
	QMenu* popMenuOnRasterLayer;	// 栅格图层节点 右键菜单
	QMenu* popMenuOnMap;	// 地图节点 右键菜单
	QMenu* menuEditFeature;	// 右键菜单 二级菜单 "Edit Feature"
};
