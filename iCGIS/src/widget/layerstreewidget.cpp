#include "widget/layerstreewidget.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDebug>
#include <QDrag>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QString>

#include "util/env.h"
#include "util/logger.h"
#include "util/appevent.h"
#include "geo/map/geomap.h"


LayersTreeWidget::LayersTreeWidget(QWidget* parent /*= nullptr*/)
    : QTreeWidget(parent), map(Env::map)
{
    this->setHeaderHidden(true);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDefaultDropAction(Qt::MoveAction);
    this->setDropIndicatorShown(true);
    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setStyleSheet("QTreeWidget::item{height:25px}");

    createMenus();
    createActions();

    // MapName
    rootItem = new QTreeWidgetItem(this);
    rootItem->setFlags(rootItem->flags() | Qt::ItemIsEditable);	// double click to rename
    rootItem->setIcon(0, QIcon("res/icons/map-2.ico"));
    rootItem->setText(0, "untitled");

    connect(this, &LayersTreeWidget::itemChanged,
            this, &LayersTreeWidget::onItemChanged);

    connect(this, &LayersTreeWidget::sigStartEditing,
            AppEvent::getInstance(), &AppEvent::onStartEditing);
    connect(this, &LayersTreeWidget::sigUpdateOpengl,
            AppEvent::getInstance(), &AppEvent::onUpdateOpengl);
    connect(this, &LayersTreeWidget::sigZoomToMap,
            AppEvent::getInstance(), &AppEvent::onZoomToMap);
    connect(this, &LayersTreeWidget::sigZoomToLayer,
            AppEvent::getInstance(), &AppEvent::onZoomToLayer);
    connect(this, &LayersTreeWidget::sigSendMapToGPU,
            AppEvent::getInstance(), &AppEvent::onSendMapToGPU);

    connect(AppEvent::getInstance(), &AppEvent::sigAddNewLayerToLayersTree,
            this, &LayersTreeWidget::onAddNewLayer);
    connect(AppEvent::getInstance(), &AppEvent::sigNewMap,
            this, &LayersTreeWidget::onNewMap);
}

LayersTreeWidget::~LayersTreeWidget()
{
}

/************************************/
/*                                  */
/*        SLOTs                     */
/*                                  */
/************************************/

// New map
void LayersTreeWidget::onNewMap(const QString& name, const QString& path) {
    if (!map->isEmpty()) {
        // save map
        int button = QMessageBox::question(this, "Prompt", "Save the project?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (button == QMessageBox::Yes) {
            // do save map
        }
    }

    delete map;
    map = new GeoMap();
    map->setName(name);
    Env::HOME = path;

    // clear all items of layers
    int childCount = rootItem->childCount();
    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem * child = rootItem->child(i);
        rootItem->removeChild(child);
        delete child;
        child = nullptr;
    }

    rootItem->setText(0, map->getName());
    emit sigUpdateOpengl();
}

// Add new layer
void LayersTreeWidget::onAddNewLayer(GeoLayer* newLayer)
{
    if (!newLayer)
        return;
    LayersTreeWidgetItem* layerItem = new LayersTreeWidgetItem();
    Qt::CheckState checked = (newLayer->isVisible()) ? Qt::Checked : Qt::Unchecked;
    layerItem->setCheckState(0, checked);
    // Allow: rename and drag
    layerItem->setFlags(layerItem->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);
    layerItem->setLID(newLayer->getLID());
    layerItem->setText(0, newLayer->getName());
    rootItem->insertChild(0, layerItem);
    this->expandAll();
}

// update forcely
void LayersTreeWidget::onUpdateLayersTree()
{
    // clear all items of layers
    int childCount = rootItem->childCount();
    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem * child = rootItem->child(i);
        rootItem->removeChild(child);
        delete child;
        child = nullptr;
    }

    rootItem->setText(0, map->getName());
    int layersCount = map->getNumLayers();
    for (int iOrder = 0; iOrder < layersCount; ++iOrder) {
        auto layer = map->getLayerByOrder(iOrder);
        LayersTreeWidgetItem* item = new LayersTreeWidgetItem(rootItem);
        Qt::CheckState checked = (layer->isVisible()) ? Qt::Checked : Qt::Unchecked;
        item->setCheckState(0, checked);
        // Allow: rename and drag
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);
        item->setLID(layer->getLID());
        item->setText(0, layer->getName());
    }

    this->expandAll();
}


/************************************/
/*                                  */
/*           Initialize             */
/*                                  */
/************************************/

// crete popMenus
void LayersTreeWidget::createMenus()
{
    popMenuOnFeatureLayer = new QMenu(this);
    popMenuOnRasterLayer = new QMenu(this);
    popMenuOnMap = new QMenu(this);
    menuEditFeature = new QMenu(tr("Edit Features"), this);
}

// create actions
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

    // save edits
    saveEditsAction = new QAction("Save Edits", this);
    saveEditsAction->setIcon(QIcon("res/icons/save.ico"));
    menuEditFeature->addAction(saveEditsAction);
    popMenuOnFeatureLayer->addMenu(menuEditFeature);
    popMenuOnMap->addMenu(menuEditFeature);
    connect(saveEditsAction, &QAction::triggered,
            this, &LayersTreeWidget::onSaveEdits);

    // stop editing
    stopEditingAction = new QAction(tr("Stop Editing"), this);
    stopEditingAction->setIcon(QIcon("res/icons/stop-editing.ico"));
    menuEditFeature->addAction(stopEditingAction);
    connect(stopEditingAction, &QAction::triggered,
            this, &LayersTreeWidget::onStopEditing);

    // Zoom to map
    zoomToMapAction = new QAction(tr("Zoom to map"), this);
    zoomToMapAction->setIcon(QIcon("res/icons/zoom-to-map.ico"));
    popMenuOnMap->addAction(zoomToMapAction);
    connect(zoomToMapAction, &QAction::triggered,
            this, &LayersTreeWidget::onZoomToMap);

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

// Double click to rename
// Default
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

void LayersTreeWidget::startDrag(Qt::DropActions/* supportedActions*/)
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
            // The current dragged item and and the item where mouse released both are layer node,
            //  and not the same node
            if (overLayerItem && currLayerItem && overLayerItem != currLayerItem) {
                int currLID = currLayerItem->getLID();
                int overLID = overLayerItem->getLID();
                rootItem->removeChild(currLayerItem);
                int insertIndex = rootItem->indexOfChild(overLayerItem);
                rootItem->insertChild(insertIndex, currLayerItem);
                // Change layer orders
                map->changeLayerOrder(currLID, overLID);
                emit sigUpdateOpengl();
            }
        }
        return;
    }

    event->ignore();
    QTreeWidget::dropEvent(event);
}

// Popup menu
void LayersTreeWidget::contextMenuEvent(QContextMenuEvent* /*event*/)
{
    QTreeWidgetItem* item = this->currentItem();

    // map node
    if (item == rootItem) {
        if (map->isEmpty()) {
            startEditingAction->setEnabled(false);
            saveEditsAction->setEnabled(false);
            stopEditingAction->setEnabled(false);
            zoomToMapAction->setEnabled(false);
        }
        else {
            if (Env::isEditing) {
                startEditingAction->setEnabled(false);
                saveEditsAction->setEnabled(true);
                stopEditingAction->setEnabled(true);
            }
            else {
                startEditingAction->setEnabled(true);
                saveEditsAction->setEnabled(false);
                stopEditingAction->setEnabled(false);
            }
            zoomToMapAction->setEnabled(true);
        }
        popMenuOnMap->exec(QCursor::pos());
    }

    // layer node
    else {
        LayersTreeWidgetItem* layerItem = toLayerItem(item);
        if (layerItem) {
            int nLID = layerItem->getLID();
            GeoLayer* layer = map->getLayerByLID(nLID);
            // feature layer
            if (layer->getLayerType() == kFeatureLayer) {
                if (Env::isEditing) {
                    startEditingAction->setEnabled(false);
                    saveEditsAction->setEnabled(true);
                    stopEditingAction->setEnabled(true);
                }
                else {
                    startEditingAction->setEnabled(true);
                    saveEditsAction->setEnabled(false);
                    stopEditingAction->setEnabled(false);
                }
                popMenuOnFeatureLayer->exec(QCursor::pos());
            }
            // raster layer
            else {
                popMenuOnRasterLayer->exec(QCursor::pos());
            }
        }
    }
}

// finish editing
void LayersTreeWidget::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QString newName = this->currentItem()->text(0);
    QTreeWidgetItem* item = this->currentItem();

    // map node
    if (item == rootItem) {
        map->setName(newName);
    }
    // layer node
    else {
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
        if (backupMap) {
            backupMap->removeLayerByLID(nLID);
        }
        emit sigUpdateOpengl();
    }
    if (map->isEmpty()) {
        Env::isEditing = false;
        emit sigStartEditing(false);
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
            attributeTableDialog = new LayerAttributeTableDialog(layer->toFeatureLayer(), this);
            QPoint pos = this->mapToGlobal(this->pos());
            //int width = this->width();
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
            styleDialog = new LayerStyleDialog(layer->toFeatureLayer(), this);
            styleDialog->setFixedSize(550, 400);
            styleDialog->show();
        }
    }
}

void LayersTreeWidget::onStartEditing()
{
    // backup map
    backupMap = map->copy();
    // Global state
    Env::isEditing = true;
    emit sigStartEditing(true);
}

void LayersTreeWidget::onSaveEdits() {
    for (auto iter = map->begin(); iter != map->end(); ++iter) {
        if ((*iter)->getLayerType() == kFeatureLayer) {
            auto layer = (*iter)->toFeatureLayer();
            layer->applyAllDeleteFlags();
            layer->updateExtent();
            layer->createGridIndex();
        }
    }
    delete backupMap;
    backupMap = map->copy();
    // clear all operations history
    Env::opList.clear();
}

void LayersTreeWidget::onStopEditing()
{
    if (!Env::opList.isAtBeginning()) {
        int button = QMessageBox::question(this, "Confirm", "Save edits?",
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (button == QMessageBox::Cancel) {
            return;
        }
        else if (button == QMessageBox::Ok) {
            delete backupMap;
            backupMap = nullptr;
        }
        else {
            delete map;
            map = backupMap;
            backupMap = nullptr;
            emit sigSendMapToGPU(false);    // not repaint now
        }
    }
    Env::isEditing = false;
    Env::opList.clear();
    map->clearSelectedFeatures();
    emit sigStartEditing(false);
    emit sigUpdateOpengl(); // update/repaint
}

void LayersTreeWidget::onZoomToMap()
{
    emit sigZoomToMap();
}

void LayersTreeWidget::onZoomToLayer()
{
    LayersTreeWidgetItem* layerItem = toLayerItem(this->currentItem());
    if (layerItem) {
        int nLID = layerItem->getLID();
        emit sigZoomToLayer(map->getLayerByLID(nLID));
    }
}

void LayersTreeWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (!map || map->isEmpty())
        return;

    // map node
    if (item->parent() != rootItem) {
        return;
    }
    // layer node
    else {
        LayersTreeWidgetItem* layerItem = toLayerItem(item);
        if (layerItem) {
            int nLID = layerItem->getLID();

            bool visible = layerItem->checkState(column) == Qt::Checked ? true : false;
            GeoLayer* layer = map->getLayerByLID(nLID);
            if (layer) {
                layer->setVisible(visible);
                emit sigUpdateOpengl();
            }
        }
    }
}
