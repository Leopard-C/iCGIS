#include "dialog/layerattributetabledialog.h"
#include "geo/map/geolayer.h"
#include "util/appevent.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

#include <set>


LayerAttributeTableDialog::LayerAttributeTableDialog(GeoFeatureLayer* layerIn, QWidget *parent)
    : QDialog(parent), layer(layerIn)
{
    Qt::WindowFlags windowFlag  = Qt::Dialog;
    windowFlag |= Qt::WindowMinimizeButtonHint;
    windowFlag |= Qt::WindowMaximizeButtonHint;
    windowFlag |= Qt::WindowCloseButtonHint;
    this->setWindowFlags(windowFlag);
    this->setWindowTitle(tr("Attribute Table"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    createToolBar();
    createWidgets();
    createActions();
    setupLayout();

    connect(this, &LayerAttributeTableDialog::sigUpdateOpengl,
            AppEvent::getInstance(), &AppEvent::onUpdateOpengl);

    readAttributeTable();
}


LayerAttributeTableDialog::~LayerAttributeTableDialog()
{
}


/*********************************/
/*                               */
/*          Initialize           */
/*                               */
/*********************************/

void LayerAttributeTableDialog::createWidgets()
{
    tableWidget = new QTableWidget(this);
    connect(tableWidget->verticalHeader(), &QHeaderView::sectionClicked,
            this, &LayerAttributeTableDialog::onSelectRows);
}

void LayerAttributeTableDialog::createActions()
{
    clearSelectedAction = new QAction(this);
    clearSelectedAction->setIcon(QIcon("res/icons/clear.ico"));
    connect(clearSelectedAction, &QAction::triggered,
            this, &LayerAttributeTableDialog::onClearSelected);
    toolBar->addAction(clearSelectedAction);

    removeRecorsdAction = new QAction(this);
    removeRecorsdAction->setIcon(QIcon("res/icons/remove.ico"));
    connect(removeRecorsdAction, &QAction::triggered,
            this, &LayerAttributeTableDialog::onRemoveSelected);
    toolBar->addAction(removeRecorsdAction);
}

// Tool bar
void LayerAttributeTableDialog::createToolBar()
{
    toolBar = new QToolBar(this);
    toolBar->setFixedHeight(20);
}

void LayerAttributeTableDialog::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(5);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(tableWidget);
}

// Read attribute table and fill in tablewidget
void LayerAttributeTableDialog::readAttributeTable()
{
    if (!layer || layer->isEmpty())
        return;

    int featuresCount = layer->getFeatureCount();
    int fieldsCount = layer->getNumFields();

    tableWidget->clear();
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setColumnCount(fieldsCount + 1);
    tableWidget->setRowCount(featuresCount);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->horizontalHeader()->setFixedHeight(20);
    tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section{background:rgb(200,200,200);}"
        );

    QStringList header;
    header << "FID";
    for (int i = 0; i < fieldsCount; ++i) {
        header << layer->getFieldDefn(i)->getName();
    }
    tableWidget->setHorizontalHeaderLabels(header);

    for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
        GeoFeature* feature = layer->getFeature(iFeature);
        if (!feature || feature->isDeleted())
            continue;

        // First col: FID
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignRight);
        item->setText(QString::number(feature->getFID()));
        tableWidget->setItem(iFeature, 0, item);

        for (int iCol = 0; iCol < fieldsCount; ++iCol) {
            QTableWidgetItem* item = new QTableWidgetItem();
            GeoFieldDefn* fieldDefn = layer->getFieldDefn(iCol);

            switch (fieldDefn->getType()) {
            default:
                break;
            case kFieldInt: {
                item->setTextAlignment(Qt::AlignRight);
                int value;
                feature->getField(iCol, &value);
                item->setText(QString::number(value));
                break;
            }
            case kFieldDouble: {
                item->setTextAlignment(Qt::AlignRight);
                double value;
                feature->getField(iCol, &value);
                item->setText(QString::number(value));
                break;
            }
            case kFieldText: {
                item->setTextAlignment(Qt::AlignLeft);
                QString value;
                feature->getField(iCol, &value);
                item->setText(value);
                break;
            }
            } // end switch

            tableWidget->setItem(iFeature, iCol + 1, item);
        } // end for iCol
    }
}

// Force to update attribute table
void LayerAttributeTableDialog::onUpdate() {
    readAttributeTable();
}

// Select rows
// highlight selected features
void LayerAttributeTableDialog::onSelectRows()
{
    auto selectedItems = tableWidget->selectedItems();

    // std::set
    // the emelem in std::set is unique
    std::set<int> rowsIndexes;

    // get all selected rows
    for (const auto& item : selectedItems) {
        rowsIndexes.insert(item->row());
    }

    std::vector<int> selectedFIDs;
    selectedFIDs.reserve(rowsIndexes.size());

    for (auto idx : rowsIndexes) {
        int nFID = tableWidget->item(idx, 0)->text().toInt();
        selectedFIDs.push_back(nFID);
    }

    layer->setSelectedFeatures(selectedFIDs);
    emit sigUpdateOpengl();
}

void LayerAttributeTableDialog::onClearSelected() {
    layer->clearSelectedFeatures();
    emit sigUpdateOpengl();
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

void LayerAttributeTableDialog::onRemoveSelected() {
    int button = QMessageBox::question(this, "Confirm", "Confirm to remove selected features?",
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (button == QMessageBox::Yes) {
        layer->deleteSelectedFeatures(false);   // hard delete
        layer->createGridIndex();
        emit sigUpdateOpengl();
        readAttributeTable();
    }
}
