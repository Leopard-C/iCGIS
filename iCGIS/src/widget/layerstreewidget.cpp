#include "layerstreewidget.h"

#include <iostream>

#include <QApplication>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QString>

#include "logger.h"
#include "widget/openglwidget.h"


LayersTreeWidget::LayersTreeWidget(GeoMap* mapIn, QWidget* parent /*= nullptr*/)
	: QTreeWidget(parent), map(mapIn)
{
	this->setHeaderHidden(true);
	this->setDragEnabled(true);
	this->setAcceptDrops(true);
	this->setDefaultDropAction(Qt::MoveAction);
	this->setDropIndicatorShown(true);
	this->setDragDropMode(QAbstractItemView::InternalMove);
	this->setStyleSheet("QTreeWidget::item{height:25px}");
	this->clear();

	createDialogs();
	createMenus();
	createActions();

	// 根节点（地图名）
	rootItem = new QTreeWidgetItem(this);
	rootItem->setFlags(rootItem->flags() | Qt::ItemIsEditable);	// 允许双击重命名
	rootItem->setIcon(0, QIcon("res/icons/map-2.ico"));
	rootItem->setText(0, "untitled");

	connect(this, &LayersTreeWidget::itemChanged, this, &LayersTreeWidget::onItemChanged);
}

LayersTreeWidget::~LayersTreeWidget()
{
}

/************************************/
/*                                  */
/*        Public Functions          */
/*                                  */
/************************************/

// 插入新的图层
void LayersTreeWidget::insertNewItem(GeoLayer* newLayer)
{
	if (!newLayer)
		return;
	LayersTreeWidgetItem* layerItem = new LayersTreeWidgetItem();
	// 默认添加进来即显示，且置为最顶层
	layerItem->setCheckState(0, Qt::Checked);
	// 允许修改图层名，和允许拖拽改变顺序
	layerItem->setFlags(layerItem->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);
	layerItem->setLID(newLayer->getLID());
	layerItem->setText(0, newLayer->getName());
	rootItem->insertChild(0, layerItem);
	this->expandAll();
}

/* 强制更新 */
/* 刷新整个LayersTreeWidget */
void LayersTreeWidget::updateLayersTree()
{
	// 删除索引图层节点
	int childCount = rootItem->childCount();
	for (int i = 0; i < childCount; ++i) {
		QTreeWidgetItem * child = rootItem->child(i);
		rootItem->removeChild(child);
		delete child;
		child = nullptr;
	}
	
	rootItem->setText(0, map->getName());
	int layersCount = map->getNumLayers();
	for (unsigned int iOrder = 0; iOrder < layersCount; ++iOrder) {
		LayersTreeWidgetItem* item = new LayersTreeWidgetItem(rootItem);
		item->setCheckState(0, Qt::Checked);
		// 允许修改图层名，和允许拖拽改变顺序
		item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);	
		item->setLID(map->getLayerByOrder(iOrder)->getLID());
		item->setText(0, map->getLayerByOrder(iOrder)->getName());
	}

	this->expandAll();
}


/************************************/
/*                                  */
/*           Initialize             */
/*                                  */
/************************************/

// 创建popMenus
void LayersTreeWidget::createMenus()
{
	popMenuOnFeatureLayer = new QMenu(this);
	popMenuOnRasterLayer = new QMenu(this);
	popMenuOnMap = new QMenu(this);
	menuEditFeature = new QMenu(tr("Edit Features"), this);
}

void LayersTreeWidget::createDialogs()
{
}

// 创建actions
void LayersTreeWidget::createActions()
{
	// rename layer or map
	renameItemAction = new QAction(tr("Rename"), this);
	renameItemAction->setIcon(QIcon("res/icons/rename.ico"));
	popMenuOnMap->addAction(renameItemAction);
	popMenuOnFeatureLayer->addAction(renameItemAction);
	popMenuOnRasterLayer->addAction(renameItemAction);
	connect(renameItemAction, &QAction::triggered, this, &LayersTreeWidget::onRenameItem);

	// remove layer (map cannot be removed)
	removeLayerAction = new QAction(tr("Remove"), this);
	removeLayerAction->setIcon(QIcon("res/icons/remove.ico"));
	popMenuOnFeatureLayer->addAction(removeLayerAction);
	popMenuOnRasterLayer->addAction(removeLayerAction);
	connect(removeLayerAction, &QAction::triggered, this, &LayersTreeWidget::onRemoveLayer);

	// show attribute table
	openAttributeTableAction = new QAction(tr("Open Attribute Table"), this);
	openAttributeTableAction->setIcon(QIcon("res/icons/table.ico"));
	popMenuOnFeatureLayer->addAction(openAttributeTableAction);
	connect(openAttributeTableAction, &QAction::triggered,
		this, &LayersTreeWidget::onOpenAttributeTable);

	// start editing
	startEditingAction = new QAction(tr("Start Editing"), this);
	startEditingAction->setIcon(QIcon("res/icons/start-editing.ico"));
	menuEditFeature->addAction(startEditingAction);
	popMenuOnFeatureLayer->addMenu(menuEditFeature);
	popMenuOnMap->addMenu(menuEditFeature);
	connect(startEditingAction, &QAction::triggered,
		this, &LayersTreeWidget::onStartEditing);

	// stop editing
	stopEditingAction = new QAction(tr("Stop Editing"), this);
	stopEditingAction->setIcon(QIcon("res/icons/stop-editing.ico"));
	menuEditFeature->addAction(stopEditingAction);
	connect(stopEditingAction, &QAction::triggered,
		this, &LayersTreeWidget::onStopEditing);

	// zoom to layer
	zoomToLayerAction = new QAction(tr("Zoom to layer"), this);
	zoomToLayerAction->setIcon(QIcon("res/icons/zoom-to-layer.ico"));
	popMenuOnFeatureLayer->addAction(zoomToLayerAction);
	popMenuOnRasterLayer->addAction(zoomToLayerAction);
	connect(zoomToLayerAction, &QAction::triggered,
		this, &LayersTreeWidget::onZoomToLayer);

	// style
	showStyleDialog = new QAction(tr("Styles"), this);
	popMenuOnFeatureLayer->addAction(showStyleDialog);
	connect(showStyleDialog, &QAction::triggered,
		this, &LayersTreeWidget::onShowStyleDialog);
}


/************************************/
/*                                  */
/*            Event                 */
/*                                  */
/************************************/

// 双击重命名
// 默认即可
//void LayerTreeWidget::mouseDoubleClickEvent(QMouseEvent *event)
//{
//	QTreeWidget::mouseDoubleClickEvent(event);
//}

void LayersTreeWidget::mousePressEvent(QMouseEvent *event)
{
	QTreeWidget::mousePressEvent(event);
}

void LayersTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
	QTreeWidget::mouseMoveEvent(event);
}

void LayersTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("drag-layer")) {
		event->acceptProposedAction();
	}
	else {
		event->ignore();
		QTreeWidget::dragEnterEvent(event);
	}
}

void LayersTreeWidget::startDrag(Qt::DropActions supportedActions)
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << "null";
	QMimeData* mimeData = new QMimeData();
	mimeData->setData("drag-layer", data);
	QDrag* drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->exec(Qt::MoveAction);
}

void LayersTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("drag-layer")) {
		QTreeWidgetItem* item = this->itemAt(event->pos());
		if (item) {
			event->acceptProposedAction();
			return;
		}
	}

	event->ignore();
	QTreeWidget::dragMoveEvent(event);
}

void LayersTreeWidget::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("drag-layer")) {
		QTreeWidgetItem* item = this->itemAt(event->pos());
		if (item) {
			LayersTreeWidgetItem* currLayerItem = toLayerItem(this->currentItem());
			LayersTreeWidgetItem* overLayerItem = toLayerItem(item);
			// 当前拖拽节点 和 松开鼠标处的节点 都是图层节点
			// 而且二者不是同一节点
			if (overLayerItem && currLayerItem && overLayerItem != currLayerItem) {
				int currLID = currLayerItem->getLID();
				int overLID = overLayerItem->getLID();
				rootItem->removeChild(currLayerItem);
				int insertIndex = rootItem->indexOfChild(overLayerItem);
				rootItem->insertChild(insertIndex, currLayerItem);
				// 改变map中的图层顺序
				map->changeLayerOrder(currLID, overLID);
				openglWidget->onLayerOrderChanged();
			}
		}
		return;
	}

	event->ignore();
	QTreeWidget::dropEvent(event);
}

// 弹出右键菜单
void LayersTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QPoint pos = this->mapFrom(this, event->pos());	// 鼠标点在该widget下的点坐标
	QTreeWidgetItem* item = this->currentItem();

	// 地图节点
	if (item == rootItem) {
		if (map->isEditable()) {
			startEditingAction->setEnabled(true);
			stopEditingAction->setEnabled(true);
		}
		else {
			startEditingAction->setEnabled(true);
			stopEditingAction->setEnabled(false);
		}
		popMenuOnMap->exec(QCursor::pos());
	}

	// 图层节点
	else {
		LayersTreeWidgetItem* layerItem = toLayerItem(item);
		if (layerItem) {
			int nLID = layerItem->getLID();
			GeoLayer* layer = map->getLayerByLID(nLID);
			// 要素图层
			if (layer->getLayerType() == kFeatureLayer) {
				if (layer->toFeatureLayer()->isEditable()) {
					startEditingAction->setEnabled(false);
					stopEditingAction->setEnabled(true);
				}
				else {
					startEditingAction->setEnabled(true);
					stopEditingAction->setEnabled(false);
				}
				popMenuOnFeatureLayer->exec(QCursor::pos());
			}
			// 栅格图层
			else {
				popMenuOnRasterLayer->exec(QCursor::pos());
			}
		}
	}
}

// 用户重命名节点完成 事件
void LayersTreeWidget::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
	QString newName = this->currentItem()->text(0);
	QTreeWidgetItem* item = this->currentItem();

	// 当前节点级别
	if (item == rootItem) {	// 地图名
		map->setName(newName);
	}
	else {		// 图层名
		LayersTreeWidgetItem* layerItem = toLayerItem(item);
		if (layerItem) {
			int nLID = layerItem->getLID();
			GeoLayer* layer = map->getLayerByLID(nLID);
			if (layer) {
				layer->setName(newName);
			}
		}
	}

	QTreeWidget::closeEditor(editor, hint);
}


/************************************/
/*                                  */
/*             slots                */
/*                                  */
/************************************/
void LayersTreeWidget::onRemoveLayer()
{
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	if (layerItem) {
		int nLID = layerItem->getLID();
		map->removeLayerByLID(nLID);
		rootItem->removeChild(layerItem);
		delete layerItem;
		openglWidget->onRemoveLayer(nLID);
	}
}

void LayersTreeWidget::onRenameItem()
{
	this->editItem(this->currentItem());
}

void LayersTreeWidget::onOpenAttributeTable()
{
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	if (layerItem) {
		int nLID = layerItem->getLID();
		GeoLayer* layer = map->getLayerByLID(nLID);
		if (layer && layer->getLayerType() == kFeatureLayer) {
			attributeTableDialog = new LayerAttributeTableDialog(layer->toFeatureLayer(), openglWidget, this);
			QPoint pos = this->mapToGlobal(this->pos());
			int width = this->width();
			int height = this->height();
			attributeTableDialog->setGeometry(pos.x(), pos.y() + height / 2, 800, 300);
			attributeTableDialog->show();
		}
	}
}

void LayersTreeWidget::onShowStyleDialog()
{
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	if (layerItem) {
		int nLID = layerItem->getLID();
		GeoLayer* layer = map->getLayerByLID(nLID);
		if (layer && layer->getLayerType() == kFeatureLayer) {
			styleDialog = new LayerStyleDialog(layer->toFeatureLayer(), openglWidget, this);
			QPoint pos = this->mapToGlobal(this->pos());
			styleDialog->setFixedSize(550, 400);
			styleDialog->show();
		}
	}
}

void LayersTreeWidget::onStartEditing()
{
	// 图层节点
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	if (layerItem) {
		int nLID = layerItem->getLID();
		GeoFeatureLayer* layer = map->getLayerByLID(nLID)->toFeatureLayer();
		layer->setEditable(true);
		openglWidget->onStartEditing(true);
	}
	// 地图节点
	else {
		for (auto it = map->begin(); it != map->end(); ++it) {
			if ((*it)->getLayerType() == kFeatureLayer)
				(*it)->toFeatureLayer()->setEditable(true);
		}
		openglWidget->onStartEditing(true);
	}
}

void LayersTreeWidget::onStopEditing()
{
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	// 图层节点
	if (layerItem) {
		int nLID = layerItem->getLID();
		GeoFeatureLayer* layer = map->getLayerByLID(nLID)->toFeatureLayer();
		layer->setEditable(false);
		if (!map->isEditable()) {
			openglWidget->onStartEditing(false);
		}
	}
	// 地图节点
	else {
		for (auto it = map->begin(); it != map->end(); ++it) {
			if ((*it)->getLayerType() == kFeatureLayer)
				(*it)->toFeatureLayer()->setEditable(false);
		}
		openglWidget->onStartEditing(false);
	}
}

void LayersTreeWidget::onZoomToLayer()
{
	LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
	if (layerItem) {
		int nLID = layerItem->getLID();
		openglWidget->onZoomToLayer(nLID);
	}
}

void LayersTreeWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
	if (!map || map->isEmpty())
		return;

	// 判断是否是根节点（地图名节点）
	if (item->parent() != rootItem) {		// 地图节点
		return;
	}
	else {		// 图层节点
		LayersTreeWidgetItem* layerItem = toLayerItem(item);
		if (layerItem) {
			int nLID = layerItem->getLID();

			// 显示 or 隐藏
			bool visable = layerItem->checkState(column) == Qt::Checked ? true : false;
			GeoLayer* layer = map->getLayerByLID(nLID);
			if (layer) {
				layer->setVisable(visable);
				openglWidget->update();
			}
		}
	}
}
