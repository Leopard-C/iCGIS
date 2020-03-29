#include "kernel_density.h"

#include "util/utility.h"
#include "util/logger.h"
#include "util/appevent.h"
#include "util/memoryleakdetect.h"
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

KernelDensityTool::KernelDensityTool(QWidget* parent /*= nullptr*/)
    : GeoTool(parent)
{
    this->setWindowTitle(tr("Kernel Density"));
    this->setWindowIcon(QIcon("res/icons/tool.ico"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setFixedSize(350, 400);
    this->setModal(true);

    setupLayout();
    initializeFill();

    connect(this, &KernelDensityTool::sigAddNewLayerToLayersTree,
            AppEvent::getInstance(), &AppEvent::onAddNewLayerToLayersTree);
    connect(this, &KernelDensityTool::sigSendLayerToGPU,
            AppEvent::getInstance(), &AppEvent::onSendLayerToGPU);
}

KernelDensityTool::~KernelDensityTool()
{
}

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

    // Enter key
    btnOK->setFocus();
    btnOK->setDefault(true);

    // Signals and slots
    connect(btnSelectFile, &QPushButton::clicked, this, &KernelDensityTool::onSetOutputRaster);
    connect(btnOK, &QPushButton::clicked, this, &KernelDensityTool::onBtnOKClicked);
    connect(btnCancel, &QPushButton::clicked, this, &KernelDensityTool::close);
}

/* Fill with default values */
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

/* Change input feature layer */
void KernelDensityTool::onChangeInputFeatures(const QString& name)
{
    GeoFeatureLayer* layer = map->getLayerByName(name)->toFeatureLayer();

    if (layer->getGeometryType() != kPoint) {
        return;
    }

    // Calculate the proper search radius
    lineEditSearchRadius->setText(QString::number(getDefaultSearchRadius(layer)));

    // Calculate the proper cell size
    lineEditOutputCellSize->setText(QString::number(getDefaultCellSize(layer)));

    // List all fields
    comboPopiField->clear();
    comboPopiField->addItem("NONE");
    int fieldCount = layer->getNumFields();
    for (int i = 0; i < fieldCount; ++i) {
        comboPopiField->addItem(layer->getFieldDefn(i)->getName());
    }
}

/* Change output file */
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
    /******************************** Get input from LineEdit **************************************/
    // inputFeatures
    GeoFeatureLayer* layer = map->getLayerByName(comboInputFeatures->currentText())->toFeatureLayer();
    // Support only for POINT Layer now.
    if (layer->getGeometryType() != kPoint) {
        QMessageBox::critical(this, "Error", "Input features are not point");
        return;
    }

    // popiField
    //GeoFieldDefn* popiFieldDefn = nullptr;
    //if (comboPopiField->currentIndex() != 0) {
    //    popiFieldDefn = layer->getFieldDefn(comboPopiField->currentText());
    //}

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

    /***********************************  Calculate KED  ****************************************/

    // Cache points
    int pointsCount = layer->getFeatureCount();
    GeoPoint** geoPoints = new GeoPoint*[pointsCount];
    for (int i = 0; i < pointsCount; ++i) {
        geoPoints[i] = layer->getFeature(i)->getGeometry()->toPoint();
    }

    // Rows and columns of output image
    GeoExtent layerExtent = layer->getExtent();
    int row = int(layerExtent.height() / cellSize + 1);
    int col = int(layerExtent.width() / cellSize + 1);

    QByteArray bytes = outputRasterFile.toLocal8Bit();
    const char* outputFile = bytes.constData();

    // Pixels data
    float* outData = new float[row * col];
    int index = 0;

    // Current grid's central point
    GeoRawPoint currPos(layerExtent.minX + cellSize / 2.0, layerExtent.maxY - cellSize / 2.0);
    double x = 0.0, y = 0.0;
    double searchRadiusSqure = searchRadius * searchRadius;

    // Progress bar
    QProgressDialog* progressDlg = new QProgressDialog(this);
    progressDlg->setAttribute(Qt::WA_DeleteOnClose, true);
    progressDlg->setOrientation(Qt::Horizontal);
    progressDlg->setWindowModality(Qt::WindowModal);
    progressDlg->setWindowTitle(tr("Kernel Density"));
    progressDlg->setLabelText(tr("Calculating......"));
    progressDlg->setCancelButtonText(tr("Cancel"));
    progressDlg->setMinimumDuration(0);
    progressDlg->setRange(0, row);

    // Hide the tool dialot
    // Show progress bar
    this->hide();

    // Calculate from TopLeft to BottomRight
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            double density = 0.0;
            for (int k = 0; k < pointsCount; ++k) {
                // Judge if the point in the enclosing squre of search area(a circle)
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
        // update progress bar
        progressDlg->setValue(i + 1);
        // whether stop progress
        if (progressDlg->wasCanceled()) {
            delete[] outData;
            delete[] geoPoints;
            this->show();
            return;
        }
    }

    progressDlg->close();

    // Open tiff file
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

    // Set projection
    OGRSpatialReference oSRS;
    char* pszSRS_WKT = nullptr;
    oSRS.SetWellKnownGeogCS("WGS84");
    oSRS.SetUTM(int((layerExtent.centerX() + 180) / 6), true);	// UTM
    oSRS.exportToWkt(&pszSRS_WKT);
    outDs->SetProjection(pszSRS_WKT);
    CPLFree(pszSRS_WKT);

    // Affine transform
    /*
[0]  top left x
[1]  w-e pixel resolution
[2]  rotation, 0 if image is "north up
[3]  top left y
[4]  rotation, 0 if image is "north up"
[5]  n-s pixel resolution (negative value)
*/
    double adfGeoTransform[6] = {
        layerExtent.minX + cellSize / 2.0, cellSize, 0,
        layerExtent.maxY - cellSize / 2.0, 0, -cellSize  // Attention! minus sign!
    };
    outDs->SetGeoTransform(adfGeoTransform);

    // Write to tiff
    GDALRasterBand* outBand = outDs->GetRasterBand(1);
    CPLErr err = outBand->RasterIO(GF_Write, 0, 0, col, row, outData, col, row, GDT_Float32, 0, 0);

    delete[] geoPoints;
    delete[] outData;
    GDALClose(outDs);

    // Write to image failed
    if (err != CPLErr::CE_None) {
        QMessageBox::critical(this, "Error", "Error occured when writing image.",
                              QMessageBox::Close);
        LError("Calculate KED: write image to file failed");
        this->close();
        return;
    }

    LInfo("Calculate KED successfully");

    // Ask users whether add the output image to current map
    auto reply = QMessageBox::question(this, tr("Option"), tr("Impot to the map?"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        GeoRasterLayer* rasterLayer = FileReader::readTiff(outputRasterFile, map);
        emit sigAddNewLayerToLayersTree(rasterLayer);
        emit sigSendLayerToGPU(rasterLayer);
    }

    this->close();
}


// Get the proper search radius
double KernelDensityTool::getDefaultSearchRadius(GeoFeatureLayer* layer)
{
    // Method 1: min(layerWidth, layerHeight) / 30

    //GeoExtent extent = layer->getExtent();
    //return std::min(extent.width(), extent.height()) / 30;


    // Moethod 2: refer to the method in ArcGIS official websit
    // ref: https://desktop.arcgis.com/en/arcmap/latest/tools/spatial-analyst-toolbox/how-kernel-density-works.htm

    int pointsCount = layer->getFeatureCount();

    // central point
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

    // The median distance and SD between the other points and the central point
    double* distancesSqure = new double[pointsCount];
    double disSqureTmp = 0.0;
    double var = 0.0;
    double x = 0.0, y = 0.0;
    for (int i = 0; i < pointsCount; ++i) {
        x = geoPoints[i]->getX();
        y = geoPoints[i]->getY();
        disSqureTmp = DIS_SQURE(x, y, centerX, centerY);
        distancesSqure[i] = disSqureTmp;
        var += disSqureTmp;
    }
    double median = sqrt(utils::getMedian(distancesSqure, pointsCount));
    delete[] distancesSqure;
    delete[] geoPoints;

    // standar deviation (SD)
    var /= pointsCount;
    double sd = sqrt(var);

    double bestSearchRadius = 0.9 * std::min(sd, sqrt(1.0 / log(2)) * median) * pow(pointsCount, -0.2);
    return bestSearchRadius;
}

// Get proper(default) cell size of output image
double KernelDensityTool::getDefaultCellSize(GeoFeatureLayer* layer)
{
    // in ArcGIS Desktop: defaultCellSize = min(layerWidth, layerHeight) / 250
    GeoExtent extent = layer->getExtent();
    return std::min(extent.width(), extent.height()) / 250;
}
