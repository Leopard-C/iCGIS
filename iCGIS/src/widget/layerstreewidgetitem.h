/*****************************************************************
** class name: LayersTreeWidgetItem
**
** description: LayersTreeWidget的子节点，继承自QTreeWidgetItem
**				添加了`LID`成员，用于表示所代表的图层
**
** last change: 2020-01-02
******************************************************************/

#pragma once

#include <QTreeWidgetItem>
#include <QTreeWidget>


class LayersTreeWidgetItem : public QTreeWidgetItem
{
public:
	LayersTreeWidgetItem(int type = 0);
	LayersTreeWidgetItem(QTreeWidget *parent, int type = 0);
	LayersTreeWidgetItem(QTreeWidgetItem *parent, int type = 0);
	~LayersTreeWidgetItem() = default;

public:
	int getLID() { return LID; }
	void setLID(int nLID) { LID = nLID; }

private:
	/* 图层名并非唯一标识一个图层 */
	/* LID才是，所以重写QTreeWidgetItem，新增一个LID成员变量 */
	int LID = 0;
};

