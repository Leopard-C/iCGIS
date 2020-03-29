/*********************************************************************
** class name:  PostgresqlTableSelect
**
** descriptio:  Selcte layer(s) in postgresql to import to the map
**
** last change: 2020-01-02
**********************************************************************/
#pragma once

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

class GeoMap;
class GeoLayer;
class GDALDataset;


class PostgresqlTableSelect : public QDialog
{
    Q_OBJECT
public:
    PostgresqlTableSelect(QWidget *parent);
    ~PostgresqlTableSelect();

public:
    void setupLayout();

signals:
    void updateCheckState(Qt::CheckState checkState);
    void sigAddNewLayerToLayersTree(GeoLayer* layer);
    void sigSendLayerToGPU(GeoLayer* layer, bool bUpdate = true);

public slots:
    void onConnectPostgresql(const QString& ip, int port, const QString& username,
                             const QString& password, const QString& database);
    void onImportLayers();
    void onCancel();
    void updateCheckStateFromHeader(Qt::CheckState checkState);
    void cellCheckboxChanged();

public:
    // layout
    QVBoxLayout* verticalLayout;	// main layout
    QHBoxLayout* horizontalLayout;

    // table widget
    QTableWidget* layersList;

    // spacerItem
    QSpacerItem* horizontalSpacer_1;
    QSpacerItem* horizontalSpacer_2;
    QSpacerItem* horizontalSpacer_3;

    // push button
    QPushButton* btnImportLayers;
    QPushButton* btnCancel;

    GDALDataset* poDS = nullptr;
    GeoMap*& map;
};
