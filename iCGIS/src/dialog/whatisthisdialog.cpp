#include "whatisthisdialog.h"

#include "geo/map/geofeature.h"
#include "geo/map/geolayer.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QString>
#include <QVBoxLayout>
#include <QStringList>
#include <QFormLayout>


WhatIsThisDialog::WhatIsThisDialog(QWidget* parent)
    : QDialog(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowTitle("What is This");
    this->setFixedHeight(500);
    this->setMinimumWidth(300);
    setupLayout();
}

void WhatIsThisDialog::setupLayout() {
    QFormLayout* formLayout = new QFormLayout();

    /* Brief Information */
    labelFID = new QLabel(this);
    QFrame* frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    labelExtentMinX = new QLabel(frame);
    labelExtentMaxX = new QLabel(frame);
    labelExtentMinY = new QLabel(frame);
    labelExtentMaxY = new QLabel(frame);
    labelGeomType = new QLabel(frame);
    labelLayerName = new QLabel(frame);

    QVBoxLayout* extentLayout = new QVBoxLayout(frame);
    QHBoxLayout* extentRow1Layout = new QHBoxLayout();
    QHBoxLayout* extentRow2Layout = new QHBoxLayout();
    QHBoxLayout* extentRow3Layout = new QHBoxLayout();
    extentRow1Layout->addStretch();
    extentRow1Layout->addWidget(labelExtentMaxY);
    extentRow1Layout->addStretch();
    extentRow2Layout->addWidget(labelExtentMinX);
    extentRow2Layout->addStretch();
    extentRow2Layout->addWidget(labelExtentMaxX);
    extentRow3Layout->addStretch();
    extentRow3Layout->addWidget(labelExtentMinY);
    extentRow3Layout->addStretch();
    extentLayout->addLayout(extentRow1Layout);
    extentLayout->addLayout(extentRow2Layout);
    extentLayout->addLayout(extentRow3Layout);

    formLayout->addRow(new QLabel("Layer Name:", this), labelLayerName);
    formLayout->addRow(new QLabel("Feature ID:", this), labelFID);
    formLayout->addRow(new QLabel("Geom Type:", this), labelGeomType);
    formLayout->addRow(new QLabel("Extent:", this), frame);

    // Detailed information [Attribute records]
    attributeTableLayout = new QHBoxLayout();
    attributeTable = new QTableWidget(this);
    attributeTable->setColumnCount(2);
    attributeTable->setHorizontalHeaderLabels(QStringList() << "Field" << "Value");
    attributeTable->setEditTriggers(QTableWidget::NoEditTriggers);
    attributeTableLayout->addWidget(attributeTable);
    attributeTableLayout->setEnabled(true);

    // Show more
    btnShowMore = new QPushButton(this);
    btnShowMore->setIcon(QIcon("res/icons/show-less.ico"));
    connect(btnShowMore, &QPushButton::clicked, this, [this]{
        if (this->attributeTable->isHidden()) {
            this->attributeTable->setHidden(false);
            this->btnShowMore->setIcon(QIcon("res/icons/show-less.ico"));
            this->setFixedHeight(500);
        }
        else {
            this->attributeTable->setHidden(true);
            this->btnShowMore->setIcon(QIcon("res/icons/show-more.ico"));
            this->setFixedHeight(0);
        }
    });

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(btnShowMore);
    mainLayout->addLayout(attributeTableLayout);
}

void WhatIsThisDialog::setFeature(GeoFeatureLayer* layer, GeoFeature* feature) {
    GeoExtent extent = feature->getExtent();
    labelFID->setText(QString::number(feature->getFID()));
    labelGeomType->setText(feature->getGeometry()->getGeometryName());
    labelExtentMinX->setText(QString::number(extent.minX));
    labelExtentMaxX->setText(QString::number(extent.maxX));
    labelExtentMinY->setText(QString::number(extent.minY));
    labelExtentMaxY->setText(QString::number(extent.maxY));
    labelLayerName->setText(layer->getName());

    /* More info */

    // FID
    attributeTable->clearContents();
    int numFields = layer->getNumFields();
    attributeTable->setRowCount(numFields + 1);
    QTableWidgetItem* fieldItem = new QTableWidgetItem();
    fieldItem->setText("FID");
    attributeTable->setItem(0, 0, fieldItem);
    QTableWidgetItem* valueItem = new QTableWidgetItem();
    valueItem->setText(QString::number(feature->getFID()));
    attributeTable->setItem(0, 1, valueItem);

    // traverse all fields
    for (int iField = 0; iField < numFields; ++iField) {
        //attributeTable->setItem(iField, 0, )
        QTableWidgetItem* fieldItem = new QTableWidgetItem();
        fieldItem->setText(feature->getFieldName(iField));
        attributeTable->setItem(iField + 1, 0, fieldItem);

        QTableWidgetItem* valueItem = new QTableWidgetItem();
        GeoFieldDefn* fieldDefn = layer->getFieldDefn(iField);

        switch (fieldDefn->getType()) {
        default:
            break;
        case kFieldInt: {
            valueItem->setTextAlignment(Qt::AlignRight);
            int value;
            feature->getField(iField, &value);
            valueItem->setText(QString::number(value));
            break;
        }
        case kFieldDouble: {
            valueItem->setTextAlignment(Qt::AlignRight);
            double value;
            feature->getField(iField, &value);
            valueItem->setText(QString::number(value));
            break;
        }
        case kFieldText: {
            valueItem->setTextAlignment(Qt::AlignLeft);
            QString value;
            feature->getField(iField, &value);
            valueItem->setText(value);
            break;
        }
        } // end switch

        attributeTable->setItem(iField + 1, 1, valueItem);
    } // end for iCol
}

void WhatIsThisDialog::closeEvent(QCloseEvent*) {
    emit closed();
}
