#include "dialog/postgresqltableselect.h"
#include "dialog/headerviewwithcheckbox.h"

#include "geo/map/geomap.h"
#include "geo/utility/geo_convert.h"
#include "geo/utility/geo_utility.h"
#include "util/logger.h"
#include "util/appevent.h"
#include "util/env.h"

#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>
#include <gdal/ogr_feature.h>
#include <gdal/ogr_geometry.h>
#include <gdal/gdal.h>

#include <QDebug>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextCodec>


PostgresqlTableSelect::PostgresqlTableSelect(QWidget *parent)
    : QDialog(parent), map(Env::map)
{
    this->setWindowTitle(tr("Select layers"));
    this->setWindowIcon(QIcon("res/icons/postgresql.ico"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    setupLayout();

    connect(this, &PostgresqlTableSelect::sigSendLayerToGPU,
            AppEvent::getInstance(), &AppEvent::onSendLayerToGPU);
    connect(this, &PostgresqlTableSelect::sigAddNewLayerToLayersTree,
            AppEvent::getInstance(), &AppEvent::onAddNewLayerToLayersTree);
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
    // intercommunication when checkstate changed
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

// Get all tables' info
// And show in QTableWidget
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

    // Connect to database
    this->poDS = (GDALDataset*)GDALOpenEx(filepath, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (!poDS) {
        LInfo("Connect to postgresql failed:{0}", filepathForLog);
        QMessageBox::critical(nullptr, "Error", "Connect to postgresql error", QMessageBox::Ok);
        return;
    }

    LInfo("Connect to postgresql successfully:{0}", filepathForLog);

    // Get layers count
    int layerCount = poDS->GetLayerCount();
    layersList->setRowCount(layerCount);
    if (0 == layerCount) {
        GDALClose(poDS);
        return;
    }

    OGRLayer* poLayer = nullptr;

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


// Click button "Import"
// import selected layer
void PostgresqlTableSelect::onImportLayers()
{
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

    for (int i = 0; i < rowCount; ++i) {
        if (!layersList->cellWidget(i, 0))
            continue;

        QCheckBox* box = (QCheckBox*)(layersList->cellWidget(i, 0)->children().at(1));
        if (!box || box->checkState() == Qt::Unchecked)
            continue;

        QTableWidgetItem* item = layersList->item(i, 1);
        QByteArray byteArray = item->text().toLocal8Bit();
        const char* layerName = byteArray.constData();

        // read layer's data and add to map
        GeoFeatureLayer* geoLayer = new GeoFeatureLayer();
        geoLayer->setName(layerName);
        if (!convertOGRLayer(poDS->GetLayerByName(layerName), geoLayer)) {
            delete geoLayer;
            continue;
        }
        map->addLayer(geoLayer);
        emit sigAddNewLayerToLayersTree(geoLayer);
        emit sigSendLayerToGPU(geoLayer);
    } // end for

    GDALClose(poDS);
    this->close();
}

void PostgresqlTableSelect::onCancel()
{
    if (poDS)
        GDALClose(poDS);
    this->close();
}

// Click the checkbox in header
// Select All or Select None
void PostgresqlTableSelect::updateCheckStateFromHeader(Qt::CheckState checkState)
{
    int rowCount = layersList->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QCheckBox* box = (QCheckBox*)(layersList->cellWidget(i, 0)->children().at(1));
        if (box) {
            box->setCheckState(checkState);
        }
    }
}

// checkstate changed in column 1
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

    // all were selected
    if (checkedCount == rowCount) {
        emit updateCheckState(Qt::Checked);
    }
    // none was selected
    else if (checkedCount == 0) {
        emit updateCheckState(Qt::Unchecked);
    }
    // portion were selected
    else {
        emit updateCheckState(Qt::PartiallyChecked);
    }
}
