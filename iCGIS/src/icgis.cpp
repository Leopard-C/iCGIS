#include "icgis.h"
#include "logger.h"

#include "geo/utility/geo_math.h"
#include "geo/utility/filereader.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QSplitter>
#include <QVBoxLayout>


ICGis::ICGis(QWidget *parent)
	: QMainWindow(parent)
{
	this->setFocusPolicy(Qt::StrongFocus);
	this->setWindowTitle(tr("iC GIS"));
	this->setWindowIcon(QIcon("res/icons/app_32x32.ico"));
	LInfo("Program start");
	map = new GeoMap();
	map->setName("untitled");

	createWidgets();
	createMenus();
	createActions();
	setupLayout();
}

ICGis::~ICGis()
{
	if (map)
		delete map;

	LInfo("Program exit");
}


/*************************************************************/
/*                                                           */
/*                       Initialize                          */
/*             1. Create actions、widgets、menu、etc         */
/*             2. Setup layout                               */
/*                                                           */
/*************************************************************/

void ICGis::createWidgets()
{
	// layers tree
	layersTreeWidget = new LayersTreeWidget(map, this);		// 左侧显示图层区域
	// tool box
	toolboxTreeWidget = new ToolBoxTreeWidget(map, this);
	// opengl
	openGLWidget = new OpenGLWidget(map, this);		// 右侧openGL绘图区域
	// search bar
	searchWidget = new GlobalSearchWidget(map, openGLWidget);	// 搜索框显示在OpenglWidget之上

	toolboxTreeWidget->setOpenGLWidget(openGLWidget);
	toolboxTreeWidget->setLayersTreeWidget(layersTreeWidget);
	layersTreeWidget->setOpenGLWidget(openGLWidget);
	searchWidget->setOpenGLWidget(openGLWidget);
}

void ICGis::createActions()
{
	// menu: File -> New
	newMapAction = new QAction(tr("Map"), this);				// file -> new -> map
	newMapAction->setIcon(QIcon("res/icons/map.ico"));
	newLayerAction = new QAction(tr("Layer"), this);			// file -> new -> layer
	newLayerAction->setIcon(QIcon("res/icons/layer.ico"));
	newFileMenu->addAction(newMapAction);
	newFileMenu->addAction(newLayerAction);

	// menu: File -> Open
	openGeoJsonMineAction = new QAction(tr("GeoJson"), this);	// file -> open -> GeoJson
	openGeoJsonMineAction->setIcon(QIcon("res/icons/geojson.ico"));
	openGeoJsonUsingGDALAction = new QAction(tr("GeoJson"), this);	// file -> open -> GeoJson
	openGeoJsonUsingGDALAction->setIcon(QIcon("res/icons/geojson.ico"));
	openShapfileAction = new QAction(tr("Shapefile"), this);	// file -> open -> Shapefile
	openShapfileAction->setIcon(QIcon("res/icons/shapefile.ico"));
	openTiffAction = new QAction(tr("Tiff"), this);
	openTiffAction->setIcon(QIcon("res/icons/tiff.ico"));
	openFileMenu->addAction(openGeoJsonMineAction);
	openFileMenu->addSeparator();
	openFileMenu->addAction(openGeoJsonUsingGDALAction);
	openFileMenu->addAction(openShapfileAction);
	openFileMenu->addAction(openTiffAction);
	connect(openGeoJsonMineAction, &QAction::triggered, this, &ICGis::onOpenGeoJsonMine);
	connect(openGeoJsonUsingGDALAction, &QAction::triggered, this, &ICGis::onOpenGeoJsonUsingGDAL);
	connect(openShapfileAction, &QAction::triggered, this, &ICGis::onOpenGeoShapefile);
	connect(openTiffAction, &QAction::triggered, this, &ICGis::onOpenTiff);

	// menu: File -> Connect
	connectPostgresqlAction = new QAction(tr("PostgreSQL"), this);
	connectPostgresqlAction->setIcon(QIcon("res/icons/postgresql.ico"));
	connectMenu->addAction(connectPostgresqlAction);
	connect(connectPostgresqlAction, &QAction::triggered, this, &ICGis::onConnectPostgresql);

	// menu: Window
	showLogDialog = new QAction(tr("Show Log"), this);
	showLogDialog->setIcon(QIcon("res/icons/log.ico"));
	windowMenu->addAction(showLogDialog);
	connect(showLogDialog, &QAction::triggered, this, &ICGis::onShowLogDialog);
}

void ICGis::createMenus()
{
	/* menu bar */
	// file
	fileMenu = menuBar()->addMenu(tr("File"));
	// file -> new
	newFileMenu = fileMenu->addMenu(tr("New"));
	newFileMenu->setIcon(QIcon("res/icons/new.ico"));
	// file -> open
	openFileMenu = fileMenu->addMenu(tr("Open"));
	openFileMenu->setIcon(QIcon("res/icons/open.ico"));
	// file -> connect (database)
	connectMenu = fileMenu->addMenu(tr("Connect"));
	connectMenu->setIcon(QIcon("res/icons/connect-db.ico"));
	// window
	windowMenu = menuBar()->addMenu(tr("Window"));

	/* pop menu */
	popMenu = new QMenu(this);
}

void ICGis::createToolBars()
{
}

// 使用代码布局ui
void ICGis::setupLayout()
{
	QWidget* centerWidget = new QWidget();
	this->setCentralWidget(centerWidget);

	searchWidget->setGeometry(20, 20, 200, 25);

	QHBoxLayout* mainLayout = new QHBoxLayout(centerWidget);
	QVBoxLayout* leftLayout = new QVBoxLayout();
	
	mainLayout->setSpacing(6);
	leftLayout->addWidget(layersTreeWidget);
	leftLayout->addWidget(toolboxTreeWidget);
	mainLayout->addLayout(leftLayout);
	mainLayout->addWidget(openGLWidget);
	mainLayout->setContentsMargins(11, 11, 11, 11);
	mainLayout->setStretch(0, 0);
	mainLayout->setStretch(1, 3);
}


// 发送数据到OpenGL并绘图
void ICGis::onUpdateOpenGLWidget()
{
	openGLWidget->update();
}


/*************************************************************/
/*                                                           */
/*                          SLOTs                            */
/*                                                           */
/*************************************************************/

// menu: file->open->GeoJson
// Method2: Using Mine
void ICGis::onOpenGeoJsonMine()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("json files(*.json)"));

	if (filepath.isEmpty()) {
		return;
	}

	GeoFeatureLayer* newLayer = FileReader::readGeoJsonMine(filepath, map);
	if (!newLayer) {
		QMessageBox::critical(this, tr("Error"), tr("Read and parse geojson error"));
		LError("Read and parse geojson file error");
		return;
	}

	LInfo("Read geojson successfully");
	layersTreeWidget->insertNewItem(newLayer);
	searchWidget->updateCompleterList();

	// 发送数据给GPU并绘图
	openGLWidget->sendDataToGPU(newLayer);
}

// menu: file->open->GeoJson
// Method2: Using GDAL
void ICGis::onOpenGeoJsonUsingGDAL()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("json files(*.json)"));

	if (filepath.isEmpty()) {
		return;
	}

	GeoFeatureLayer* newFeatureLayer = FileReader::readGeoJsonUsingGDAL(filepath, map);
	if (!newFeatureLayer) {
		QMessageBox::critical(this, tr("Error"), tr("Read and parse geojson error"));
		LError("Read and parse geojson file error");
		return;
	}

	LInfo("Read geojson successfully");
	layersTreeWidget->insertNewItem(newFeatureLayer);
	searchWidget->updateCompleterList();

	// 发送数据给GPU并绘图
	openGLWidget->sendDataToGPU(newFeatureLayer);
}


// menu: file->open->Shapefile
void ICGis::onOpenGeoShapefile()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("shapefile(*.shp)"));

	if (filepath.isEmpty())
		return;

	GeoFeatureLayer* newFeatureLayer = FileReader::readShapefile(filepath, map);
	if (!newFeatureLayer) {
		QMessageBox::critical(this, tr("Error"), tr("Read and parse shapefile error"));
		LError("Read and parse shapefile error");
		return;
	}

	LInfo("Read and parse shapefile successfully");
	layersTreeWidget->insertNewItem(newFeatureLayer);
	searchWidget->updateCompleterList();

	// 发送数据给GPU并绘图
	openGLWidget->sendDataToGPU(newFeatureLayer);
}

// file->open->Tiff
void ICGis::onOpenTiff()
{
	QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("TIFF(*.tif)"));

	if (filepath.isEmpty())
		return;

	GeoRasterLayer* newRasterLayer = FileReader::readTiff(filepath, map);
	if (!newRasterLayer) {
		QMessageBox::critical(this, tr("Error"), tr("Read and parse shapefile error"));
		LError("Read and parse shapefile error");
		return;
	}

	LInfo("Read and parse shapefile successfully");
	layersTreeWidget->insertNewItem(newRasterLayer);
	searchWidget->updateCompleterList();

	// 发送数据给GPU并绘图
	openGLWidget->sendDataToGPU(newRasterLayer);
}

// file->connect->Postgresql
void ICGis::onConnectPostgresql()
{
	postgresqlConnectDialog = new PostgresqlConnect(this);
	postgresqlConnectDialog->setFixedSize(280, 200);
	postgresqlConnectDialog->setModal(true);

	postgresqlTableSelectDialog = new PostgresqlTableSelect(this);
	postgresqlTableSelectDialog->setFixedSize(500, 300);
	postgresqlTableSelectDialog->setModal(true);
	postgresqlTableSelectDialog->map = this->map;
	postgresqlTableSelectDialog->setOpenglWidget(openGLWidget);
	postgresqlTableSelectDialog->setLayersTreeWidget(layersTreeWidget);

	// 两个子窗口的通信
	// 连接数据库 和 读取数据库
	connect(postgresqlConnectDialog, &PostgresqlConnect::btnConnectClicked,
		postgresqlTableSelectDialog, &PostgresqlTableSelect::onConnectPostgresql);

	postgresqlConnectDialog->show();
}

void ICGis::onShowLogDialog()
{
	viewLogDialog = new ViewLog(this);
	viewLogDialog->setFixedSize(800, 300);
	viewLogDialog->setModal(false);
	viewLogDialog->show();
}

void ICGis::contextMenuEvent(QContextMenuEvent *event)
{
}

