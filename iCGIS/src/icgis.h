/*******************************************************
** class name: GisDev
**
** description: Main Window
**
** last change: 2020-03-28
*******************************************************/

#pragma once

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <map>

#include "dialog/postgresqlconnect.h"
#include "dialog/postgresqltableselect.h"
#include "dialog/viewlog.h"
#include "widget/globalsearchwidget.h"
#include "widget/layerstreewidget.h"
#include "widget/openglwidget.h"
#include "widget/statusbar.h"
#include "widget/toolbar.h"
#include "widget/toolboxtreewidget.h"
#include "geo/map/geomap.h"


class ICGis : public QMainWindow {
    Q_OBJECT
public:
    ICGis(QWidget* parent = nullptr);
    ~ICGis() override;

public:
    /* Initialize */
    void createWidgets();
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void setupLayout();

    void test_geo_math();

public slots:
    void onNewMap();
    void onOpenGeoJsonUsingGDAL();
    void onOpenGeoJsonMine();
    void onOpenGeoShapefile();
    void onOpenTiff();
    void onConnectPostgresql();
    void onShowLogDialog();
    void onAbout();

protected:
    virtual void closeEvent(QCloseEvent*) override;

private:
    GeoMap*& map;

    /* Dialog */
    PostgresqlConnect* postgresqlConnectDialog;
    PostgresqlTableSelect* postgresqlTableSelectDialog;
    ViewLog* viewLogDialog;

    /* Widget */
    LayersTreeWidget* layersTreeWidget;
    ToolBoxTreeWidget* toolboxTreeWidget;
    OpenGLWidget* openGLWidget;
    GlobalSearchWidget* searchWidget;

    /* Tool bar */
    ToolBar* toolbar;

    /* Status bar */
    StatusBar* statusbar;

    /* MenuBar */
    QMenu* fileMenu;      // File
    QMenu* newFileMenu;   //  File -> New
    QMenu* openFileMenu;  //  File -> Open
    QMenu* connectMenu;   //  File -> Connect
    QMenu* windowMenu;    // Window
    QMenu* aboutMenu;     // About

    /* Right clicked menu */
    QMenu* popMenu;

    /* Action */
    QAction* openGeoJsonUsingGDALAction;
    QAction* openGeoJsonMineAction;
    QAction* openShapfileAction;
    QAction* openTiffAction;
    QAction* connectPostgresqlAction;
    QAction* newMapAction;
    QAction* newLayerAction;
    QAction* showLogDialog;
    QAction* aboutAction;
};
