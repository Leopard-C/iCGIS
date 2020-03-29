/*****************************************************************
** class name: LayersTreeWidgetItem
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
    int getLID() const { return LID; }
    void setLID(int nLID) { LID = nLID; }

private:
    // Layer's unique ID
    int LID = 0;
};

