/*******************************************************
** class name:  LayerAttributeTableDialog
**
** description: Layer's attribute table
**
** last change: 2020-03-28
*******************************************************/
#pragma once

#include <QAction>
#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>

class GeoFeatureLayer;


class LayerAttributeTableDialog : public QDialog
{
    Q_OBJECT
public:
    LayerAttributeTableDialog(GeoFeatureLayer* layerIn, QWidget *parent);
    ~LayerAttributeTableDialog();

signals:
    void sigUpdateOpengl();

public slots:
    void onUpdate();
    void onSelectRows();
    void onRemoveSelected();
    void onClearSelected();

public:
    void createWidgets();
    void createActions();
    void createToolBar();
    void setupLayout();

private:
    void readAttributeTable();

private:
    GeoFeatureLayer* layer;

    // widgets
    QTableWidget* tableWidget;

    // toolBar
    QToolBar* toolBar;

    // actions
    QAction* clearSelectedAction;
    QAction* removeRecorsdAction;
};
