#include "icgis.h"

#include "util/env.h"
#include "util/utility.h"
#include "util/logger.h"
#include "geo/utility/filereader.h"
#include "geo/utility/geo_math.h"
#include "dialog/aboutdialog.h"
#include "dialog/newmapdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QMetaType>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QVBoxLayout>


ICGis::ICGis(QWidget *parent)
    : QMainWindow(parent), map(Env::map)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->setWindowTitle(tr("iC GIS"));
    this->setWindowIcon(QIcon("res/icons/app_32x32.ico"));

    qRegisterMetaType<GeoFeature>("GeoFeature&");
    qRegisterMetaType<GeoLayer>("GeoLayer");
    qRegisterMetaType<GeoFeatureLayer>("GeoFeatureLayer");
    qRegisterMetaType<GeoRasterLayer>("GeoRasterLayer");

    QString logFilePath = QDir::currentPath() + "/logs";
    QDir dir(logFilePath);
    if(!dir.exists()) {
        dir.mkdir(logFilePath);
    }

    LInfo("Program start");
    Env::map->setName("untitled");

    createMenus();
    createStatusBar();
    createToolBar();
    createWidgets();
    createActions();
    setupLayout();
}

ICGis::~ICGis() {
    if (map)
        delete map;
    if (statusbar)
        delete statusbar;

    LInfo("Program exit");
}

/*************************************************************/
/*                                                           */
/*                       Initialize                          */
/*             1. Create actions, widgets, menu, etc         */
/*             2. Setup layout                               */
/*                                                           */
/*************************************************************/

void ICGis::createMenus() {
    /* menu bar */
    // File
    fileMenu = menuBar()->addMenu(tr("File"));
    // File -> New
    newFileMenu = fileMenu->addMenu(tr("New"));
    newFileMenu->setIcon(QIcon("res/icons/new.ico"));
    // File -> Open
    openFileMenu = fileMenu->addMenu(tr("Open"));
    openFileMenu->setIcon(QIcon("res/icons/open.ico"));
    // File -> Connect (database)
    connectMenu = fileMenu->addMenu(tr("Connect"));
    connectMenu->setIcon(QIcon("res/icons/connect-db.ico"));
    // Window
    windowMenu = menuBar()->addMenu(tr("Window"));
    // about
    aboutMenu = menuBar()->addMenu(tr("About"));

    /* pop menu */
    popMenu = new QMenu(this);
}

void ICGis::createToolBar() {
    toolbar = new ToolBar(this);
    this->addToolBar(toolbar);
}

void ICGis::createStatusBar() {
    this->statusBar()->setStyleSheet("QStatusBar::item{border: 0px}");
    statusbar = new StatusBar(this->statusBar());
}

void ICGis::createWidgets() {
    // layers tree
    layersTreeWidget = new LayersTreeWidget(this);
    // tool box
    toolboxTreeWidget = new ToolBoxTreeWidget(this);
    // opengl
    openGLWidget = new OpenGLWidget(this);
    // search bar
    searchWidget =
        new GlobalSearchWidget(openGLWidget);
}


void ICGis::createActions() {
    // menu: File -> New
    newMapAction = new QAction(tr("Map"), this); // file -> new -> map
    newMapAction->setIcon(QIcon("res/icons/map.ico"));
    newLayerAction = new QAction(tr("Layer"), this); // file -> new -> layer
    newLayerAction->setIcon(QIcon("res/icons/layer.ico"));
    newFileMenu->addAction(newMapAction);
    newFileMenu->addAction(newLayerAction);
    connect(newMapAction, &QAction::triggered, this, &ICGis::onNewMap);

    // menu: File -> Open
    openGeoJsonMineAction =
        new QAction(tr("GeoJson"), this); // file -> open -> GeoJson
    openGeoJsonMineAction->setIcon(QIcon("res/icons/geojson.ico"));
    openGeoJsonUsingGDALAction =
        new QAction(tr("GeoJson"), this); // file -> open -> GeoJson
    openGeoJsonUsingGDALAction->setIcon(QIcon("res/icons/geojson.ico"));
    openShapfileAction =
        new QAction(tr("Shapefile"), this); // file -> open -> Shapefile
    openShapfileAction->setIcon(QIcon("res/icons/shapefile.ico"));
    openTiffAction = new QAction(tr("Tiff"), this);
    openTiffAction->setIcon(QIcon("res/icons/tiff.ico"));
    openFileMenu->addAction(openGeoJsonMineAction);
    openFileMenu->addSeparator();
    openFileMenu->addAction(openGeoJsonUsingGDALAction);
    openFileMenu->addAction(openShapfileAction);
    openFileMenu->addAction(openTiffAction);
    connect(openGeoJsonMineAction, &QAction::triggered, this,
            &ICGis::onOpenGeoJsonMine);
    connect(openGeoJsonUsingGDALAction, &QAction::triggered, this,
            &ICGis::onOpenGeoJsonUsingGDAL);
    connect(openShapfileAction, &QAction::triggered, this,
            &ICGis::onOpenGeoShapefile);
    connect(openTiffAction, &QAction::triggered, this, &ICGis::onOpenTiff);

    // menu: File -> Connect
    connectPostgresqlAction = new QAction(tr("PostgreSQL"), this);
    connectPostgresqlAction->setIcon(QIcon("res/icons/postgresql.ico"));
    connectMenu->addAction(connectPostgresqlAction);
    connect(connectPostgresqlAction, &QAction::triggered, this,
            &ICGis::onConnectPostgresql);

    // menu: Window
    showLogDialog = new QAction(tr("Show Log"), this);
    showLogDialog->setIcon(QIcon("res/icons/log.ico"));
    windowMenu->addAction(showLogDialog);
    connect(showLogDialog, &QAction::triggered, this, &ICGis::onShowLogDialog);

    // menu: About
    aboutAction = new QAction(tr("About iC GIS"), this);
    aboutAction->setIcon(QIcon(""));
    aboutMenu->addAction(aboutAction);
    connect(aboutAction, &QAction::triggered, this, &ICGis::onAbout);
}


// layout
void ICGis::setupLayout() {
    QWidget *centerWidget = new QWidget();
    this->setCentralWidget(centerWidget);

    searchWidget->setGeometry(20, 20, 200, 25);

    QHBoxLayout *mainLayout = new QHBoxLayout(centerWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout();

    mainLayout->setSpacing(6);
    leftLayout->addWidget(layersTreeWidget);
    leftLayout->addWidget(toolboxTreeWidget);
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(openGLWidget);
    mainLayout->setContentsMargins(11, 11, 11, 11);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 3);
}


/*************************************************************/
/*                                                           */
/*                          SLOTs                            */
/*                                                           */
/*************************************************************/

// menu: file->new->map
void ICGis::onNewMap() {
    NewMapDialog* dialog = new NewMapDialog(this);
    dialog->setModal(true);
    dialog->show();
}


// menu: file->open->GeoJson
// Method1: Parse GeoJson file using JsonCpp library
void ICGis::onOpenGeoJsonMine() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Open File"), "", tr("json files(*.json)"), nullptr,
        QFileDialog::DontUseNativeDialog);

    if (files.isEmpty()) {
        return;
    }

    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        GeoFeatureLayer *newFeatureLayer = FileReader::readGeoJsonMine(*iter, map);
        if (!newFeatureLayer) {
            QString errmsg = "Read and parse geoJson failed: " + *iter;
            QMessageBox::critical(this, "Error", errmsg, QMessageBox::Ok);
            LError(errmsg.toStdString());
            continue;
        }
        layersTreeWidget->onAddNewLayer(newFeatureLayer);
        openGLWidget->onSendFeatureLayerToGPU(newFeatureLayer, false);    // not update immediately
    }

    searchWidget->updateCompleterList();
    openGLWidget->update();
}

// menu: file->open->GeoJson
// Method2: Using GDAL
void ICGis::onOpenGeoJsonUsingGDAL() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Open File"), "", tr("json files(*.json)"), nullptr,
        QFileDialog::DontUseNativeDialog);

    if (files.isEmpty()) {
        return;
    }

    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        GeoFeatureLayer *newFeatureLayer = FileReader::readGeoJsonUsingGDAL(*iter, map);
        if (!newFeatureLayer) {
            QString errmsg = "Read and parse geoJson failed: " + *iter;
            QMessageBox::critical(this, "Error", errmsg, QMessageBox::Ok);
            LError(errmsg.toStdString());
            continue;
        }
        layersTreeWidget->onAddNewLayer(newFeatureLayer);
        openGLWidget->onSendFeatureLayerToGPU(newFeatureLayer, false);    // not update immediately
    }

    searchWidget->updateCompleterList();
    openGLWidget->update();
}

// menu: file->open->Shapefile
void ICGis::onOpenGeoShapefile() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Open File"), "", tr("shapefile(*.shp)"), nullptr,
        QFileDialog::DontUseNativeDialog);

    if (files.isEmpty())
        return;

    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        GeoFeatureLayer *newFeatureLayer = FileReader::readShapefile(*iter, map);
        if (!newFeatureLayer) {
            QString errmsg = "Read and parse shapefile failed: " + *iter;
            QMessageBox::critical(this, "Error", errmsg, QMessageBox::Ok);
            LError(errmsg.toStdString());
            continue;
        }
        layersTreeWidget->onAddNewLayer(newFeatureLayer);
        openGLWidget->onSendFeatureLayerToGPU(newFeatureLayer, false);    // not update immediately
    }

    searchWidget->updateCompleterList();
    openGLWidget->update();
}

// file->open->Tiff
void ICGis::onOpenTiff() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Open File"), "", tr("TIFF(*.tif)"), nullptr,
        QFileDialog::DontUseNativeDialog);

    if (files.isEmpty())
        return;

    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        GeoRasterLayer *newRasterLayer = FileReader::readTiff(*iter, map);
        if (!newRasterLayer) {
            QString errmsg = "Read and parse TIFF image failed: " + *iter;
            QMessageBox::critical(this, "Error", errmsg, QMessageBox::Ok);
            LError(errmsg.toStdString());
            return;
        }
        layersTreeWidget->onAddNewLayer(newRasterLayer);
        openGLWidget->onSendRasterLayerToGPU(newRasterLayer, false); // not update immediately
    }

    searchWidget->updateCompleterList();
    openGLWidget->update();
}

// file->connect->Postgresql
void ICGis::onConnectPostgresql() {
    postgresqlConnectDialog = new PostgresqlConnect(this);
    postgresqlConnectDialog->setFixedSize(280, 200);
    postgresqlConnectDialog->setModal(true);

    postgresqlTableSelectDialog = new PostgresqlTableSelect(this);
    postgresqlTableSelectDialog->setFixedSize(500, 300);
    postgresqlTableSelectDialog->setModal(true);
    postgresqlTableSelectDialog->map = this->map;

    connect(postgresqlConnectDialog, &PostgresqlConnect::btnConnectClicked,
            postgresqlTableSelectDialog, &PostgresqlTableSelect::onConnectPostgresql);

    postgresqlConnectDialog->show();
}

void ICGis::onShowLogDialog() {
    viewLogDialog = new ViewLog(this);
    viewLogDialog->setFixedSize(800, 300);
    viewLogDialog->setModal(false);
    viewLogDialog->show();
}

void ICGis::onAbout() {
    AboutDialog* aboutDialog = new AboutDialog(this);
    aboutDialog->setModal(true);
    aboutDialog->show();
}

void ICGis::closeEvent(QCloseEvent *event) {
    int button = QMessageBox::question(this, "Prompt", "Save the project?",
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (button == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    else if (button == QMessageBox::Yes) {

    }
    else {
        QMainWindow::closeEvent(event);
    }
}
