#include "dialog/postgresqltableselect.h"

#include "dialog/headerviewwithcheckbox.h"
#include "geo/geometry/geogeometry.h"
#include "geo/map/geolayer.h"
#include "geo/utility/geo_convert.h"
#include "geo/utility/geo_utility.h"
#include "widget/openglwidget.h"
#include "widget/layerstreewidget.h"
#include "logger.h"

#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>
#include <gdal/ogr_feature.h>
#include <gdal/ogr_geometry.h>

#include <QDebug>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextCodec>
#include <iostream>

PostgresqlTableSelect::PostgresqlTableSelect(QWidget *parent)
	: QDialog(parent)
{
	this->setWindowTitle(tr("Select layers"));
	this->setWindowIcon(QIcon("res/icons/postgresql.ico"));
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	setupLayout();
}

PostgresqlTableSelect::~PostgresqlTableSelect()
{
}


void PostgresqlTableSelect::setupLayout()
{
	this->setWindowTitle("Select layers");

	layersList = new QTableWidget();
	layersList->setColumnCount(4);
	layersList->setSelectionBehavior(QAbstractItemView::SelectRows);
	// header with check box
	HeaderViewWithCheckbox* header = new HeaderViewWithCheckbox(0, Qt::Horizontal, layersList);
	header->setStretchLastSection(true);
	header->setStyleSheet("alignment:left;");
	// 互相通信，checkbox状态改变
	connect(header, SIGNAL(headCheckboxToggled(Qt::CheckState)), 
		this, SLOT(updateCheckStateFromHeader(Qt::CheckState)));
	connect(this, SIGNAL(updateCheckState(Qt::CheckState)),
		header, SLOT(updateCheckState(Qt::CheckState)));
	layersList->setHorizontalHeader(header);
	layersList->setHorizontalHeaderLabels(QStringList() << "" << "Name" << "FeatureCount" << "GeometryType");
	layersList->setColumnWidth(0, 20);
	layersList->setColumnWidth(1, 200);
	layersList->setColumnWidth(2, 100);
	layersList->setColumnWidth(3, 100);
	layersList->setShowGrid(false);

	// push button
	btnImportLayers = new QPushButton("Import");
	btnCancel = new QPushButton("Cancel");
	connect(btnImportLayers, SIGNAL(clicked()),
		this, SLOT(onImportLayers()));
	connect(btnCancel, SIGNAL(clicked()),
		this, SLOT(onCancel()));

	horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout = new QHBoxLayout();
	horizontalLayout->setSpacing(6);
	horizontalLayout->addItem(horizontalSpacer_1);
	horizontalLayout->addWidget(btnImportLayers);
	horizontalLayout->addItem(horizontalSpacer_2);
	horizontalLayout->addWidget(btnCancel);
	horizontalLayout->addItem(horizontalSpacer_3);

	// main layout
	verticalLayout = new QVBoxLayout(this);
	verticalLayout->setSpacing(6);
	verticalLayout->addWidget(layersList);
	verticalLayout->addLayout(horizontalLayout);
}

// 连接数据库，并显示所有的table（即图层）
// 显示到QTableWidget控件中，供用户选择
void PostgresqlTableSelect::onConnectPostgresql(const QString& ip, int port, const QString& username, const QString& password, const QString& database)
{
	OGRRegisterAll();
	char filepath[128] = { 0 };
	QByteArray bytesIp = ip.toLocal8Bit();
	QByteArray bytesUserName = username.toLocal8Bit();
	QByteArray bytesPassword = password.toLocal8Bit();
	QByteArray bytesDatabase = database.toLocal8Bit();
	sprintf(filepath, "PG:dbname=%s host=%s port=%d user=%s password=%s",
		bytesDatabase.constData(),
		bytesIp.constData(),
		port,
		bytesUserName.constData(),
		bytesPassword.constData()
	);

	char filepathForLog[128] = { 0 };
	sprintf(filepathForLog, "PG:dbname=%s host=%s port=%d user=%s password=******",
		bytesDatabase.constData(),
		bytesIp.constData(),
		port,
		bytesUserName.constData()
	);

	// 连接数据库
	this->poDS = (GDALDataset*)GDALOpenEx(filepath, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (!poDS) {
		LInfo("Connect to postgresql failed:{0}", filepathForLog);
		QMessageBox::critical(nullptr, "Error", "Connect to postgresql error", QMessageBox::Ok);
		return;
	}

	LInfo("Connect to postgresql successfully:{0}", filepathForLog);

	// 获取图层数量
	int layerCount = poDS->GetLayerCount();
	layersList->setRowCount(layerCount);
	if (0 == layerCount) {
		GDALClose(poDS);
		return;
	}

	OGRLayer* poLayer = nullptr;

	// 遍历所有layer（即postgresql数据库中的数据表）
	for (int i = 0; i < layerCount; ++i) {
		poLayer = poDS->GetLayer(i);
		//poLayer->ResetReading();

		// first column: checkbox
		QCheckBox* checkBox = new QCheckBox();
		QHBoxLayout* hLayout = new QHBoxLayout();
		QWidget* widget = new QWidget(layersList);
		hLayout->addWidget(checkBox);
		hLayout->setMargin(0);
		hLayout->setAlignment(checkBox, Qt::AlignCenter);
		widget->setLayout(hLayout);
		checkBox->setCheckState(Qt::Unchecked);
		layersList->setCellWidget(i, 0, widget);
		connect(checkBox, SIGNAL(stateChanged(int)), 
			this, SLOT(cellCheckboxChanged()));

		// second column: layer name
		QTableWidgetItem* layerName = new QTableWidgetItem();
		layerName->setFlags(layerName->flags() & (~Qt::ItemIsEditable));
		layerName->setTextAlignment(Qt::AlignCenter);
		layerName->setText(poLayer->GetName());
		layersList->setItem(i, 1, layerName);

		// third column: feature bandsCount
		QTableWidgetItem* featureCount = new QTableWidgetItem();
		featureCount->setFlags(featureCount->flags() & (~Qt::ItemIsEditable));
		featureCount->setTextAlignment(Qt::AlignCenter);
		featureCount->setText(QString::number(poLayer->GetFeatureCount()));
		layersList->setItem(i, 2, featureCount);

		// fourth column: feature geometry type
		QTableWidgetItem* geometryType = new QTableWidgetItem();
		geometryType->setFlags(featureCount->flags() & (~Qt::ItemIsEditable));
		geometryType->setTextAlignment(Qt::AlignCenter);
		geometryType->setText(wkbTypeToString(poLayer->GetGeomType()));
		layersList->setItem(i, 3, geometryType);
	}
	layersList->show();
	this->show();

	return;
}


// 点击import按钮
// 导入选中的图层
void PostgresqlTableSelect::onImportLayers()
{
	std::cout << "here" << std::endl;
	if (!poDS) {
		QMessageBox::critical(this, "Error", "Not connected to server", QMessageBox::Ok);
		return;
	}

	if (!map) {
		QMessageBox::critical(this, "Error", "There is no map to add the layers!", QMessageBox::Ok);
		return;
	}

	int rowCount = layersList->rowCount();
	if (rowCount == 0) {
		return;
	}

	// 遍历layerList，找出用户选中的图层，然后导入
	for (int i = 0; i < rowCount; ++i) {
		if (!layersList->cellWidget(i, 0))
			continue;

		QCheckBox* box = (QCheckBox*)(layersList->cellWidget(i, 0)->children().at(1));
		if (!box || box->checkState() == Qt::Unchecked)
			continue;

		QTableWidgetItem* item = layersList->item(i, 1);	// 第二列，存储layer名称
		QByteArray byteArray = item->text().toLocal8Bit();
		const char* layerName = byteArray.constData();

		// 读取图层数据到GeoFeatureLayer， 并添加到map中
		GeoFeatureLayer* geoLayer = new GeoFeatureLayer();
		geoLayer->setName(layerName);
		if (!convertOGRLayer(poDS->GetLayerByName(layerName), geoLayer)) {
			delete geoLayer;
			continue;
		}
		map->addLayer(geoLayer);
		layersTreeWidget->insertNewItem(geoLayer);
		openglWidget->sendDataToGPU(geoLayer);
	} // end for

	GDALClose(poDS);
	this->close();
}

void PostgresqlTableSelect::onCancel()
{
	if (poDS)
		GDALClose(poDS);

	this->close();	// 关闭本对话框
}

// 点击表头的复选框
// 全选 or 全不选
void PostgresqlTableSelect::updateCheckStateFromHeader(Qt::CheckState checkState)
{
	int rowCount = layersList->rowCount();
	for (int i = 0; i < rowCount; ++i) {
		// 获取第一列的QCheckBox
		//qDebug() << layersList->cellWidget(i, 0)->children();
		QCheckBox* box = (QCheckBox*)(layersList->cellWidget(i, 0)->children().at(1));
		// 下面一行行不通
		// 因为cellWidget(i, 0)不仅仅有一个QCheckBox，
		// 还有一个QHLayout用于控制QCheckBox的居中
		// 从上面qDebug()输出两个children可以看出
		//QCheckBox* box = qobject_cast<QCheckBox*>(layersList->cellWidget(i, 0));
		if (box) {
			box->setCheckState(checkState);
		}
	}
}

// 第一列（非表头）checkbox 状态改变
void PostgresqlTableSelect::cellCheckboxChanged()
{
	int rowCount = layersList->rowCount();
	if (rowCount == 0)
		return;
	int checkedCount = 0;
	for (int i = 0; i < rowCount; ++i) {
		if (!layersList->cellWidget(i, 0))
			continue;
		QCheckBox* box = (QCheckBox*)(layersList->cellWidget(i, 0)->children().at(1));
		if (box && box->checkState() == Qt::Checked)
			checkedCount++;
	}

	// 全选中
	if (checkedCount == rowCount) {
		emit updateCheckState(Qt::Checked);
	}
	else if (checkedCount == 0) {	// 一个都没选中
		emit updateCheckState(Qt::Unchecked);
	}
	else {		// 部分选中
		emit updateCheckState(Qt::PartiallyChecked);
	}
}


bool PostgresqlTableSelect::parseWkbPolygon(OGRPolygon* poPolygon, GeoFeature* geoFeature)
{
	OGRPoint ptTemp;

	int numInteriorRings = poPolygon->getNumInteriorRings();

	GeoPolygon* geoPolygon = new GeoPolygon();
	geoPolygon->reserveInteriorRingsCount(numInteriorRings + 1);

	OGRLinearRing* poExteriorRing = poPolygon->getExteriorRing();
	GeoLinearRing* geoExteriorRing = new GeoLinearRing();
	int numExteriorRingPoints = poExteriorRing->getNumPoints();

	for (int k = 0; k < numExteriorRingPoints; ++k) {
		poExteriorRing->getPoint(k, &ptTemp);
		geoExteriorRing->addPoint(ptTemp.getX(), ptTemp.getY());
	}
	geoPolygon->setExteriorRing(geoExteriorRing);

	for (int i = 0; i < numInteriorRings; ++i) {
		OGRLinearRing* poInteriorRing = poPolygon->getInteriorRing(i);
		GeoLinearRing* geoInteriorRing = new GeoLinearRing();

		int numInteriorRingPoints = poInteriorRing->getNumPoints();
		for (int k = 0; k < numInteriorRingPoints; ++i) {
			poInteriorRing->getPoint(k, &ptTemp);
			geoInteriorRing->addPoint(ptTemp.getX(), ptTemp.getY());
		}
		geoPolygon->addInteriorRing(geoInteriorRing);
	}

	return true;
}

bool PostgresqlTableSelect::parseWkbMultipoint(OGRMultiPoint* poMultiPoint, GeoFeature* geoFeature)
{
	return true;
}

bool PostgresqlTableSelect::parseWkbMultipolygon(OGRMultiPolygon* poMultipolygon, GeoFeature* getFeature)
{
	OGRPolygon* poPolygon = nullptr;
	int polygonCount = poMultipolygon->getNumGeometries();
	for (int i = 0; i < polygonCount; ++i) {
		poPolygon = (OGRPolygon*)poMultipolygon->getGeometryRef(i);
		OGRLinearRing* linerRing = (OGRLinearRing*)poPolygon->getExteriorRing();
		GeoPolygon polygon;

	}

	return true;
}
