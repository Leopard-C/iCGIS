#include "kernel_density.h"

#include "utility.h"
#include "memoryleakdetect.h"
#include "geo/utility/filereader.h"

#include <algorithm>
#include <iostream>
#include <cmath>

#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QProgressDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>

#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <thread>

KernelDensityTool::KernelDensityTool(GeoMap* mapIn, QWidget* parent /*= nullptr*/)
	: GeoTool(mapIn, parent)
{
	// 关闭即销毁
	this->setWindowTitle(tr("Kernel Density"));
	this->setWindowIcon(QIcon("res/icons/tool.ico"));
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	this->setFixedSize(350, 400);
	this->setModal(true);

	setupLayout();
	initializeFill();

	this->show();
}

KernelDensityTool::~KernelDensityTool()
{
}

/* 布局 */
void KernelDensityTool::setupLayout()
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QLabel* label1 = new QLabel(tr("Input point or polyline features"));
	comboInputFeatures = new QComboBox();
	mainLayout->addWidget(label1);
	mainLayout->addWidget(comboInputFeatures);
	connect(comboInputFeatures, &QComboBox::currentTextChanged,
		this, &KernelDensityTool::onChangeInputFeatures);

	QLabel* label2 = new QLabel(tr("Population field"));
	comboPopiField = new QComboBox();
	mainLayout->addWidget(label2);
	mainLayout->addWidget(comboPopiField);

	QLabel* label3 = new QLabel(tr("Output raster"));
	lineEditOutputRaster = new QLineEdit();
	QPushButton* btnSelectFile = new QPushButton();
	btnSelectFile->setIcon(QIcon("res/icons/open.ico"));
	QHBoxLayout* hLayout1 = new QHBoxLayout();
	hLayout1->addWidget(lineEditOutputRaster);
	hLayout1->addWidget(btnSelectFile);
	mainLayout->addWidget(label3);
	mainLayout->addLayout(hLayout1);

	QLabel* label4 = new QLabel(tr("Output cell size (optional)"));
	lineEditOutputCellSize = new QLineEdit();
	lineEditOutputCellSize->setAlignment(Qt::AlignRight);
	mainLayout->addWidget(label4);
	mainLayout->addWidget(lineEditOutputCellSize);

	QLabel* label5 = new QLabel(tr("Search radius (optional)"));
	lineEditSearchRadius = new QLineEdit();
	lineEditSearchRadius->setAlignment(Qt::AlignRight);
	mainLayout->addWidget(label5);
	mainLayout->addWidget(lineEditSearchRadius);

	QLabel* label6 = new QLabel(tr("Area units (optional)"));
	comboAreaUnits = new QComboBox();
	mainLayout->addWidget(label6);
	mainLayout->addWidget(comboAreaUnits);

	QLabel* label7 = new QLabel(tr("output values are (optional)"));
	comboOutputValuesType = new QComboBox();
	mainLayout->addWidget(label7);
	mainLayout->addWidget(comboOutputValuesType);

	QLabel* label8 = new QLabel(tr("Method (optiona)"));
	comboMethod = new QComboBox();
	mainLayout->addWidget(label8);
	mainLayout->addWidget(comboMethod);

	QPushButton* btnOK = new QPushButton("OK");
	QPushButton* btnCancel = new QPushButton(tr("Cancel"));
	QSpacerItem* spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QSpacerItem* spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QSpacerItem* spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QHBoxLayout* hLayout2 = new QHBoxLayout();
	hLayout2->addItem(spacerItem1);
	hLayout2->addWidget(btnOK);
	hLayout2->addItem(spacerItem2);
	hLayout2->addWidget(btnCancel);
	hLayout2->addItem(spacerItem3);
	mainLayout->addLayout(hLayout2);

	/* 回车键 */
	btnOK->setFocus();
	btnOK->setDefault(true);

	/* Signals and slots */
	connect(btnSelectFile, &QPushButton::clicked, this, &KernelDensityTool::onSetOutputRaster);
	connect(btnOK, &QPushButton::clicked, this, &KernelDensityTool::onBtnOKClicked);
	connect(btnCancel, &QPushButton::clicked, this, &KernelDensityTool::close);
}

/* 填充默认值 */
void KernelDensityTool::initializeFill()
{
	int layersCount = map->getNumLayers();
	for (int i = 0; i < layersCount; ++i) {
		GeoLayer* layer = map->getLayerById(i);
		if (layer->getLayerType() == kFeatureLayer) {
			comboInputFeatures->addItem(layer->getName());
		}
	}
}

/* 用户选择了一个输入图层 */
void KernelDensityTool::onChangeInputFeatures(const QString& name)
{
	GeoFeatureLayer* layer = map->getLayerByName(name)->toFeatureLayer();

	// 暂时只计算点图层
	if (layer->getGeometryType() != kPoint) {
		return;
	}

	// 计算最为合适的search radius
	lineEditSearchRadius->setText(QString::number(getDefaultSearchRadius(layer)));

	// 设置默认cell size
	lineEditOutputCellSize->setText(QString::number(getDefaultCellSize(layer)));

	// 列出layer的所有field
	comboPopiField->clear();
	comboPopiField->addItem("NONE");
	int fieldCount = layer->getNumFields();
	for (int i = 0; i < fieldCount; ++i) {
		comboPopiField->addItem(layer->getFieldDefn(i)->getName());
	}
}

/* 设置输出栅格文件路径 */
void KernelDensityTool::onSetOutputRaster()
{
	QString filepath = QFileDialog::getSaveFileName(this, tr("Set output raster file"), ".", "TIFF File(*.tif)");
	lineEditOutputRaster->setText(filepath);
}

/****************************************/
/*                                      */
/*     Run                              */ 
/*       Calculate kernel density       */
/*       Output a geoTiff file          */
/*                                      */
/****************************************/
void KernelDensityTool::onBtnOKClicked()
{
	/********************************  获取输入框内的内容  **************************************/
	// inputFeatures
	GeoFeatureLayer* layer = map->getLayerByName(comboInputFeatures->currentText())->toFeatureLayer();
	// 暂时只支持计算点的核密度
	if (layer->getGeometryType() != kPoint) {
		QMessageBox::critical(this, "Error", "Input features are not point");
		return;
	}

	// popiField
	GeoFieldDefn* popiFieldDefn = nullptr;
	if (comboPopiField->currentIndex() != 0) {
		popiFieldDefn = layer->getFieldDefn(comboPopiField->currentText());
	}

	// outputRaster
	QString outputRasterFile = lineEditOutputRaster->text();
	if (outputRasterFile.isEmpty()) {
		QMessageBox::critical(this, "Error", "Ouput raster file can't be empty");
		return;
	}

	// output cell size
	double cellSize = 0.0;
	if (lineEditOutputCellSize->text().isEmpty()) {
		cellSize = getDefaultCellSize(layer);
	}
	else {
		cellSize = lineEditOutputCellSize->text().toDouble();
	}

	// search radius
	double searchRadius = 0.0;
	if (!lineEditSearchRadius->text().isEmpty())
		searchRadius = lineEditSearchRadius->text().toDouble();

	/***********************************  开始计算核密度  ****************************************/

	// 缓存points
	int pointsCount = layer->getFeatureCount();
	GeoPoint** geoPoints = new GeoPoint*[pointsCount];
	for (int i = 0; i < pointsCount; ++i) {
		geoPoints[i] = layer->getFeature(i)->getGeometry()->toPoint();
	}

	// 栅格行列数
	GeoExtent layerExtent = layer->getExtent();
	int row = layerExtent.height() / cellSize + 1;
	int col = layerExtent.width() / cellSize + 1;

	QByteArray bytes = outputRasterFile.toLocal8Bit();
	const char* outputFile = bytes.constData();

	// 栅格图像像素值
	float* outData = new float[row * col];
	int index = 0;

	// 当前栅格中心点的位置
	// 起点是左上角第一个像素
	GeoRawPoint currPos(layerExtent.minX + cellSize / 2.0, layerExtent.maxY - cellSize / 2.0);
	double x = 0.0, y = 0.0;
	double cellSizeSqure = cellSize * cellSize;
	double searchRadiusSqure = searchRadius * searchRadius;

	/* 进度条 */
	QProgressDialog* progressDlg = new QProgressDialog(this);
	progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
	progressDlg->setOrientation(Qt::Horizontal);
	progressDlg->setWindowModality(Qt::WindowModal);
	progressDlg->setWindowTitle(tr("Kernel Density"));
	progressDlg->setLabelText(tr("Calculating......"));
	progressDlg->setCancelButtonText(tr("Cancel"));
	progressDlg->setMinimumDuration(0);
	progressDlg->setRange(0, row);

	// 隐藏当前窗口
	// 显示进度条
	this->hide();

	// 遍历栅格
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			double density = 0.0;
			for (int k = 0; k < pointsCount; ++k) {
				// 先判断点是否在以currPos为中心，以searchRadius为边长的正方形内
				x = geoPoints[k]->getX();
				y = geoPoints[k]->getY();
				if (fabs(x - currPos.x) < searchRadius && fabs(y - currPos.y) < searchRadius) {
					double disSqure = DIS_SQURE(x, y, currPos.x, currPos.y);
					if (disSqure < searchRadiusSqure) {
						density += pow(1 - disSqure / searchRadiusSqure, 2);
					}
				}
			}
			currPos.x += cellSize;
			density = density * 3.0 / (PI * searchRadiusSqure);
			outData[index++] = density;
		}
		currPos.x = layerExtent.minX + cellSize / 2.0;
		currPos.y -= cellSize;
		// 设置进度条进度
		progressDlg->setValue(i + 1);
		// 用户是否点击了取消
		if (progressDlg->wasCanceled()) {
			delete[] outData;
			delete[] geoPoints;
			this->show();
			return;
		}
	}

	progressDlg->close();

	// 输出tif文件
	GDALAllRegister();
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (!poDriver) {
		delete[] outData;
		delete[] geoPoints;
		return;
	}

	char** papszOptions = nullptr;
	papszOptions = CSLSetNameValue(papszOptions, "TILED", "YES");
	papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "PACKBITS");
	GDALDataset* outDs = poDriver->Create(outputFile, col, row, 1, GDT_Float32, papszOptions);
	CPLFree(papszOptions);

	// 设置投影
	OGRSpatialReference oSRS;
	char* pszSRS_WKT = nullptr;
	oSRS.SetWellKnownGeogCS("WGS84");
	oSRS.SetUTM((layerExtent.centerX() + 180) / 6, true);	// UTM投影带号
	oSRS.exportToWkt(&pszSRS_WKT);
	outDs->SetProjection(pszSRS_WKT);
	CPLFree(pszSRS_WKT);

	// 设置仿射变换参数（即添加地理坐标信息）
	/*
		[0]  top left x 左上角x坐标
		[1]  w-e pixel resolution 东西方向像素分辨率
		[2]  rotation, 0 if image is "north up" 旋转角度，正北向上时为0
		[3]  top left y 左上角y坐标
		[4]  rotation, 0 if image is "north up" 旋转角度，正北向上时为0
		[5]  n-s pixel resolution 南北向像素分辨率
		x/y为图像的x/y坐标，geox/geoy为对应的投影坐标
	*/
	double adfGeoTransform[6] = {
		layerExtent.minX + cellSize / 2.0, cellSize, 0, 
		layerExtent.maxY - cellSize / 2.0, 0, -cellSize  // 注意负号，否则图像上下颠倒
	};	
	outDs->SetGeoTransform(adfGeoTransform);

	// 写入Tiff
	GDALRasterBand* outBand = outDs->GetRasterBand(1);
	outBand->RasterIO(GF_Write, 0, 0, col, row, outData, col, row, GDT_Float32, 0, 0);
	
	// 清理内存
	// 关闭GDAL数据集（写入文件，RasterIO函数并没有写入文件）
	delete[] geoPoints;
	delete[] outData;
	GDALClose(outDs);

	// 是否添加Tiff图像到地图
	auto reply = QMessageBox::question(this, tr("Option"), tr("Impot to the map?"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {
		GeoRasterLayer* rasterLayer = FileReader::readTiff(outputRasterFile, map);
		emit addedTiffToMap(rasterLayer);
	}
	
	this->close();
}


// 计算最佳的搜索半径
double KernelDensityTool::getDefaultSearchRadius(GeoFeatureLayer* layer)
{
	/* 方式一：Arcgis核密度分析的默认值：点图层边界width和height的较小者的1/30 */

	//GeoExtent extent = layer->getExtent();
	//return std::min(extent.width(), extent.height()) / 30;


	/* 方式二：Arcgis官网提供的计算searchRadis方法 */
	// ref: https://desktop.arcgis.com/en/arcmap/latest/tools/spatial-analyst-toolbox/how-kernel-density-works.htm

	// 点要素图层，要素的数目即为点的数目
	int pointsCount = layer->getFeatureCount();

	// 计算 中心点
	double sumX = 0.0, sumY = 0.0;
	GeoPoint** geoPoints = new GeoPoint*[pointsCount];
	for (int i = 0; i < pointsCount; ++i) {
		geoPoints[i] = layer->getFeature(i)->getGeometry()->toPoint();
		sumX += geoPoints[i]->getX();
		sumY += geoPoints[i]->getY();
	}
	double centerX = sumX / pointsCount;
	double centerY = sumY / pointsCount;
	//GeoRawPoint center(centerX, centerY);

	// 距离中心点的中位数, 标准差
	double* distancesSqure = new double[pointsCount];
	double disSqureTmp = 0.0;
	double var = 0.0;
	double x = 0.0, y = 0.0;
	for (int i = 0; i < pointsCount; ++i) {
		x = geoPoints[i]->getX();
		y = geoPoints[i]->getY();
		disSqureTmp = DIS_SQURE(x, y, centerX, centerY);
		distancesSqure[i] = disSqureTmp;		// 这里用的是距离的平方，因为这不会影响最后得到中位数（距离平方的中位数再开平方即可）
		var += disSqureTmp;
	}
	double median = sqrt(utils::getMedian(distancesSqure, pointsCount));
	delete[] distancesSqure;
	delete[] geoPoints;

	// 距离中心点的标准差
	var /= pointsCount;
	double sd = sqrt(var);

	// 计算最佳搜索半径
	double bestSearchRadius = 0.9 * std::min(sd, sqrt(1.0 / log(2)) * median) * pow(pointsCount, -0.2);

	return bestSearchRadius;
}


double KernelDensityTool::getDefaultCellSize(GeoFeatureLayer* layer)
{
	// Arcgis核密度分析 cell size 的默认值为：点图层边界width和height的较小者的1/250
	GeoExtent extent = layer->getExtent();
	return std::min(extent.width(), extent.height()) / 250;
}

