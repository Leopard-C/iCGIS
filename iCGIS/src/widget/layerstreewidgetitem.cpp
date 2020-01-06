#include "layerstreewidgetitem.h"



LayersTreeWidgetItem::LayersTreeWidgetItem(int type /*= 0*/)
	: QTreeWidgetItem(type)
{

}

LayersTreeWidgetItem::LayersTreeWidgetItem(QTreeWidget *parent, int type /*= 0*/)
	: QTreeWidgetItem(parent, type)
{

}

LayersTreeWidgetItem::LayersTreeWidgetItem(QTreeWidgetItem *parent, int type /*= 0*/)
	: QTreeWidgetItem(parent, type)
{

}
